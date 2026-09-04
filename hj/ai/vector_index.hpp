#ifndef VECTOR_INDEX_HPP
#define VECTOR_INDEX_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>
#include <system_error>
#include <stdexcept>
#include <cstdint>

#include <faiss/Index.h>
#include <faiss/IndexFlat.h>
#include <faiss/IndexIDMap.h>
#include <faiss/clone_index.h>
#include <faiss/impl/AuxIndexStructures.h>
#include <faiss/impl/io.h>
#include <faiss/index_factory.h>
#include <faiss/index_io.h>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <process.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace hj
{
using vindex_flat_t    = faiss::IndexFlat;
using vindex_flat_ip_t = faiss::IndexFlatIP;
using vindex_flat_l2_t = faiss::IndexFlatL2;
using vindex_idmap_t   = faiss::IndexIDMap;

using vindex_dimension_t           = typename faiss::idx_t;
using vindex_idx_t                 = typename faiss::idx_t;
using vindex_count_t               = typename faiss::idx_t;
using vindex_range_search_result_t = typename faiss::RangeSearchResult;

enum class vector_index_errc
{
    success = 0,

    invalid_argument,
    null_index,
    out_of_range,

    type_mismatch,
    unsupported_operation,
    not_trained,
    dimension_mismatch,
    capacity_exceeded,

    serialization_error,
    file_not_found,
    file_empty,
    io_error,
    permission_denied,

    faiss_exception
};

namespace detail
{

inline auto process_id() noexcept
{
#if defined(_WIN32)
    return ::_getpid();
#else
    return ::getpid();
#endif
}

inline bool sync_file(const std::string &path) noexcept
{
#if defined(_WIN32)
    std::wstring wpath = std::filesystem::path(path).wstring();
    HANDLE       hFile = CreateFileW(wpath.c_str(),
                                     GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        return false;
    bool success = FlushFileBuffers(hFile);
    CloseHandle(hFile);
    return success;
#else
    int fd = ::open(path.c_str(), O_WRONLY);
    if(fd < 0)
        return false;
    bool success = (::fsync(fd) == 0);
    ::close(fd);
    return success;
#endif
}

inline void sync_parent_dir(const std::string &filepath) noexcept
{
#if !defined(_WIN32)
    std::filesystem::path p(filepath);
    std::string           parent_dir =
        p.parent_path().empty() ? "." : p.parent_path().string();
    int dir_fd = ::open(parent_dir.c_str(), O_RDONLY | O_DIRECTORY);
    if(dir_fd >= 0)
    {
        ::fsync(dir_fd);
        ::close(dir_fd);
    }
#endif
}

inline bool atomic_rename(const std::string &temp_path,
                          const std::string &target_path) noexcept
{
#if defined(_WIN32)
    std::wstring wtemp   = std::filesystem::path(temp_path).wstring();
    std::wstring wtarget = std::filesystem::path(target_path).wstring();

    return MoveFileExW(wtemp.c_str(),
                       wtarget.c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)
           != 0;
#else
    std::error_code ec;
    std::filesystem::rename(temp_path, target_path, ec);
    return !ec;
#endif
}

class vector_index_category final : public std::error_category
{
  public:
    const char *name() const noexcept override { return "hj::vector_index"; }

    std::string message(int ev) const override
    {
        switch(static_cast<vector_index_errc>(ev))
        {
            case vector_index_errc::success:
                return "Success";
            case vector_index_errc::invalid_argument:
                return "Invalid argument (nullptr or illegal parameter)";
            case vector_index_errc::null_index:
                return "Underlying index pointer is nullptr";
            case vector_index_errc::out_of_range:
                return "Index or ID out of range";
            case vector_index_errc::type_mismatch:
                return "Index C++ type downcast failed (type mismatch)";
            case vector_index_errc::unsupported_operation:
                return "Operation not supported by the current underlying "
                       "index type";
            case vector_index_errc::not_trained:
                return "Underlying index is not trained yet";
            case vector_index_errc::dimension_mismatch:
                return "Input vector dimension mismatch";
            case vector_index_errc::capacity_exceeded:
                return "Buffer or storage capacity exceeded";
            case vector_index_errc::serialization_error:
                return "Serialization or deserialization failed";
            case vector_index_errc::file_not_found:
                return "File does not exist";
            case vector_index_errc::file_empty:
                return "File is empty or corrupted";
            case vector_index_errc::io_error:
                return "I/O operation failed (read/write/sync/rename error)";
            case vector_index_errc::permission_denied:
                return "Permission denied for file or directory operation";
            case vector_index_errc::faiss_exception:
                return "Internal Faiss exception caught";
            default:
                return "Unknown index error";
        }
    }
};

inline const std::error_category &index_category() noexcept
{
    static vector_index_category category;
    return category;
}

} // namespace hj::detail

inline std::error_code make_error_code(vector_index_errc e) noexcept
{
    return std::error_code(static_cast<int>(e), detail::index_category());
}

template <typename T = faiss::Index>
class vector_index
{
  public:
    using err_t = std::error_code;

    enum class metric
    {
        inner_product = faiss::METRIC_INNER_PRODUCT,
        l2            = faiss::METRIC_L2,
        l1            = faiss::METRIC_L1,
        linf          = faiss::METRIC_Linf,
        lp            = faiss::METRIC_Lp,

        canberra       = faiss::METRIC_Canberra,
        bray_curtis    = faiss::METRIC_BrayCurtis,
        jensen_shannon = faiss::METRIC_JensenShannon,

        jaccard       = faiss::METRIC_Jaccard,
        nan_euclidean = faiss::METRIC_NaNEuclidean,
        gower         = faiss::METRIC_GOWER,
    };

  public:
    vector_index()
        : _index(nullptr)
    {
    }

    explicit vector_index(T *idx)
        : _index(idx)
    {
    }

    explicit vector_index(std::unique_ptr<T> idx)
        : _index(std::move(idx))
    {
    }

    vector_index(const vector_index &)            = delete;
    vector_index &operator=(const vector_index &) = delete;

    vector_index(vector_index &&other) noexcept
    {
        std::unique_lock<std::shared_mutex> lock(other._rw_mutex);
        _index = std::move(other._index);
    }

    vector_index &operator=(vector_index &&other) noexcept
    {
        if(this != &other)
        {
            std::unique_lock<std::shared_mutex> lock_this(_rw_mutex,
                                                          std::defer_lock);
            std::unique_lock<std::shared_mutex> lock_other(other._rw_mutex,
                                                           std::defer_lock);
            std::lock(lock_this, lock_other);

            _index = std::move(other._index);
        }
        return *this;
    }

    ~vector_index() = default;

    vindex_dimension_t dimension() const noexcept
    {
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        return _index ? _index->d : 0;
    }

    metric metric_type() const noexcept
    {
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        return _index ? static_cast<metric>(_index->metric_type) : metric::l2;
    }

    bool is_trained() const noexcept
    {
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        return _index ? _index->is_trained : false;
    }

    bool empty() const noexcept
    {
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        return !_index || _index->ntotal == 0;
    }

    vindex_count_t total() const
    {
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        return _index ? _index->ntotal : 0;
    }

    void reset()
    {
        std::unique_lock<std::shared_mutex> lock(_rw_mutex);
        if(_index)
            _index->reset();
    }

    [[deprecated("Use with_index() for thread-safety, or unsafe_get_index() if "
                 "you explicitly handle synchronization.")]]
    T *get_index()
    {
        return _index.get();
    }
    [[deprecated(
        "Use with_index_read() for thread-safety, or unsafe_get_index() if you "
        "explicitly handle synchronization.")]]
    const T *get_index() const
    {
        return _index.get();
    }
    T       *unsafe_get_index() noexcept { return _index.get(); }
    const T *unsafe_get_index() const noexcept { return _index.get(); }

    /**
     * @brief Provides exclusive (write-locked) thread-safe access to the underlying FAISS index.
     * 
     * @tparam F Callable type (lambda, function object, function pointer).
     * @param fn Callback function accepting `T&` as its parameter.
     * @return decltype(auto) The return value of the callback function `fn`.
     * 
     * @warning **Deadlock Warning (Alien Code Contract)**:
     * DO NOT invoke any public member functions of this `vector_index` instance (e.g., `this->search()`, 
     * `this->total()`, `this->add()`) inside the callback. Doing so will result in a recursive locking 
     * deadlock because `std::shared_mutex` is non-recursive.
     * 
     * @note **Correct Usage**:
     * Access and operate directly on the raw `T&` reference passed into the callback.
     * 
     * @code{.cpp}
     * // ✅ Correct: Directly manipulate the inner FAISS index reference
     * index.with_index([](auto& raw_faiss_idx) {
     *     auto count = raw_faiss_idx.ntotal; // Safe: Direct access to underlying member
     *     raw_faiss_idx.reset();             // Safe: Direct call to FAISS API
     * });
     * 
     * // ❌ Incorrect: Calling member methods of vector_index inside the callback causes DEADLOCK!
     * index.with_index([&](auto& raw_faiss_idx) {
     *     auto total = index.total();        // DEADLOCK! Re-acquires shared_lock while holding unique_lock
     * });
     * @endcode
     */
    template <typename F>
    decltype(auto) with_index(F &&fn)
    {
        std::unique_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            throw std::runtime_error(
                "vector_index: underlying index is nullptr");

        return std::forward<F>(fn)(*_index);
    }

    /**
     * @brief Provides shared (read-locked) thread-safe access to the underlying FAISS index.
     * 
     * @tparam F Callable type (lambda, function object, function pointer).
     * @param fn Callback function accepting `const T&` as its parameter.
     * @return decltype(auto) The return value of the callback function `fn`.
     * 
     * @warning **Deadlock Warning (Alien Code Contract)**:
     * DO NOT invoke any public member functions of this `vector_index` instance (e.g., `this->search()`, 
     * `this->total()`) inside the callback. Re-entering read-lock routines while a writer thread is 
     * waiting will trigger thread starvation deadlocks.
     * 
     * @note **Correct Usage**:
     * Access and operate directly on the const raw `const T&` reference passed into the callback.
     * 
     * @code{.cpp}
     * // ✅ Correct: Read directly from the inner FAISS index reference
     * index.with_index_read([](const auto& raw_faiss_idx) {
     *     auto count = raw_faiss_idx.ntotal;
     *     // Perform custom batch operations directly on raw_faiss_idx
     * });
     * @endcode
     */
    template <typename F>
    decltype(auto) with_index_read(F &&fn) const
    {
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            throw std::runtime_error(
                "vector_index: underlying index is nullptr");

        return std::forward<F>(fn)(*static_cast<const T *>(_index.get()));
    }

    void set_index(T *idx)
    {
        std::unique_lock<std::shared_mutex> lock(_rw_mutex);
        _index.reset(idx);
    }

    void set_index(std::unique_ptr<T> idx)
    {
        std::unique_lock<std::shared_mutex> lock(_rw_mutex);
        _index = std::move(idx);
    }

    template <typename... Args>
    void build(Args &&...args)
    {
        std::unique_lock<std::shared_mutex> lock(_rw_mutex);
        _index = std::make_unique<T>(std::forward<Args>(args)...);
    }

    std::error_code
    build_factory(int dim, const char *description, metric m = metric::l2)
    {
        if(dim <= 0 || !description)
            return vector_index_errc::invalid_argument;

        std::unique_lock<std::shared_mutex> lock(_rw_mutex);
        try
        {
            faiss::Index *raw_idx =
                faiss::index_factory(dim,
                                     description,
                                     static_cast<faiss::MetricType>(m));
            if(!raw_idx)
                return vector_index_errc::faiss_exception;

            T *typed_idx = dynamic_cast<T *>(raw_idx);
            if(!typed_idx)
            {
                delete raw_idx;
                return vector_index_errc::type_mismatch;
            }

            _index.reset(typed_idx);
            return vector_index_errc::success;
        }
        catch(const std::exception &)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code train(vindex_count_t vectors_num, const float *vectors)
    {
        if(!vectors || vectors_num <= 0)
            return vector_index_errc::invalid_argument;

        std::unique_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(_index->d <= 0)
            return vector_index_errc::invalid_argument;

        if(_index->is_trained)
            return vector_index_errc::success;

        try
        {
            _index->train(vectors_num, vectors);
            return vector_index_errc::success;
        }
        catch(const faiss::FaissException &)
        {
            return vector_index_errc::faiss_exception;
        }
        catch(const std::exception &)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code train(const std::vector<float> &vectors)
    {
        if(vectors.empty())
            return vector_index_errc::invalid_argument;

        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(_index->d <= 0)
            return vector_index_errc::invalid_argument;

        const size_t dim = static_cast<size_t>(_index->d);
        if(vectors.size() % dim != 0)
            return vector_index_errc::dimension_mismatch;

        const vindex_count_t vectors_num =
            static_cast<vindex_count_t>(vectors.size() / dim);

        lock.unlock();
        return train(vectors_num, vectors.data());
    }

    std::error_code add(vindex_count_t vectors_num,
                        const float   *vectors,
                        size_t         vectors_capacity)
    {
        if(!vectors || vectors_num <= 0)
            return vector_index_errc::invalid_argument;

        std::unique_lock<std::shared_mutex> lock(_rw_mutex);
        return _add(vectors_num, vectors, vectors_capacity);
    }

    std::error_code add(const std::vector<float> &vectors)
    {
        if(vectors.empty())
            return vector_index_errc::invalid_argument;

        std::unique_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(_index->d <= 0)
            return vector_index_errc::invalid_argument;

        const size_t dim = static_cast<size_t>(_index->d);
        if(vectors.size() % dim != 0)
            return vector_index_errc::dimension_mismatch;

        const vindex_count_t vectors_num =
            static_cast<vindex_count_t>(vectors.size() / dim);

        return _add(vectors_num, vectors.data(), vectors.size());
    }

    std::error_code add_with_ids(vindex_count_t      vectors_num,
                                 const float        *vectors,
                                 size_t              vectors_capacity,
                                 const vindex_idx_t *ids,
                                 size_t              ids_capacity)
    {
        if(!vectors || !ids || vectors_num <= 0)
            return vector_index_errc::invalid_argument;

        std::unique_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(_index->d <= 0)
            return vector_index_errc::invalid_argument;

        if(!_index->is_trained)
            return vector_index_errc::not_trained;

        const size_t req_n = static_cast<size_t>(vectors_num);
        const size_t dim   = static_cast<size_t>(_index->d);

        if(req_n > (std::numeric_limits<size_t>::max)() / dim)
            return vector_index_errc::out_of_range;

        if(vectors_capacity < req_n * dim || ids_capacity < req_n)
            return vector_index_errc::capacity_exceeded;

        try
        {
            _index->add_with_ids(vectors_num, vectors, ids);
            return vector_index_errc::success;
        }
        catch(const faiss::FaissException &)
        {
            return vector_index_errc::unsupported_operation;
        }
        catch(const std::exception &)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code merge_from(const vector_index<T> &other,
                               vindex_idx_t           add_id = 0) noexcept
    {
        if(this == &other)
            return vector_index_errc::invalid_argument;

        std::unique_lock<std::shared_mutex> lock_this(_rw_mutex,
                                                      std::defer_lock);
        std::shared_lock<std::shared_mutex> lock_other(other._rw_mutex,
                                                       std::defer_lock);
        std::lock(lock_this, lock_other);

        if(!_index || !other._index)
            return vector_index_errc::null_index;

        if(!_index->is_trained || !other._index->is_trained)
            return vector_index_errc::not_trained;

        if(_index->d != other._index->d)
            return vector_index_errc::dimension_mismatch;

        try
        {
            _index->merge_from(*other._index, add_id);
            return vector_index_errc::success;
        }
        catch(const faiss::FaissException &)
        {
            return vector_index_errc::unsupported_operation;
        }
        catch(...)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code reconstruct(vindex_idx_t key, float *vector) const
    {
        if(!vector)
            return vector_index_errc::invalid_argument;
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;
        return _reconstruct(key, vector);
    }

    std::error_code reconstruct_batch(vindex_count_t      n,
                                      const vindex_idx_t *keys,
                                      float              *vectors,
                                      size_t vectors_capacity) const
    {
        if(!keys || !vectors || n <= 0)
            return vector_index_errc::invalid_argument;

        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(_index->d <= 0)
            return vector_index_errc::invalid_argument;

        const size_t req_n = static_cast<size_t>(n);
        const size_t dim   = static_cast<size_t>(_index->d);

        if(req_n > (std::numeric_limits<size_t>::max)() / dim)
            return vector_index_errc::out_of_range;

        if(vectors_capacity < req_n * dim)
            return vector_index_errc::capacity_exceeded;

        return _reconstruct_batch(n, keys, vectors);
    }

    std::error_code reconstruct_n(vindex_idx_t   i0,
                                  vindex_count_t ni,
                                  float         *vectors,
                                  size_t         vectors_capacity) const
    {
        if(!vectors || ni <= 0)
            return vector_index_errc::invalid_argument;

        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(_index->d <= 0)
            return vector_index_errc::invalid_argument;

        const size_t req_ni = static_cast<size_t>(ni);
        const size_t dim    = static_cast<size_t>(_index->d);

        if(req_ni > (std::numeric_limits<size_t>::max)() / dim)
            return vector_index_errc::out_of_range;

        if(vectors_capacity < req_ni * dim)
            return vector_index_errc::capacity_exceeded;

        return _reconstruct_n(i0, ni, vectors);
    }

    std::error_code reconstruct_batch(vindex_count_t      n,
                                      const vindex_idx_t *keys,
                                      std::vector<float> &vectors) const
    {
        if(!keys || n <= 0)
            return vector_index_errc::invalid_argument;

        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(_index->d <= 0)
            return vector_index_errc::invalid_argument;

        const size_t req_n   = static_cast<size_t>(n);
        const size_t dim     = static_cast<size_t>(_index->d);
        const size_t max_cap = vectors.max_size();

        if(req_n > max_cap / dim)
            return vector_index_errc::out_of_range;

        try
        {
            vectors.resize(req_n * dim);
        }
        catch(...)
        {
            return vector_index_errc::capacity_exceeded;
        }

        return _reconstruct_batch(n, keys, vectors.data());
    }

    std::error_code search(vindex_count_t vectors_num,
                           const float   *vectors,
                           vindex_count_t ret_num,
                           float         *distances,
                           vindex_idx_t  *indexs) const
    {
        if(!vectors || !distances || !indexs || vectors_num <= 0
           || ret_num <= 0)
            return vector_index_errc::invalid_argument;

        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(!_index->is_trained)
            return vector_index_errc::not_trained;

        try
        {
            _index->search(vectors_num, vectors, ret_num, distances, indexs);
            return vector_index_errc::success;
        }
        catch(...)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code range_search(vindex_count_t                vectors_num,
                                 const float                  *vectors,
                                 float                         radius,
                                 vindex_range_search_result_t *result) const
    {
        if(!vectors || !result || vectors_num <= 0)
            return vector_index_errc::invalid_argument;

        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(!_index->is_trained)
            return vector_index_errc::not_trained;

        try
        {
            _index->range_search(vectors_num, vectors, radius, result);
            return vector_index_errc::success;
        }
        catch(...)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code serialize(std::vector<uint8_t> &buffer) const
    {
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        try
        {
            faiss::VectorIOWriter writer;
            faiss::write_index(_index.get(), &writer);
            buffer = std::move(writer.data);
            return vector_index_errc::success;
        }
        catch(const faiss::FaissException &)
        {
            return vector_index_errc::serialization_error;
        }
        catch(...)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code deserialize(std::vector<uint8_t> &&buffer)
    {
        if(buffer.empty())
            return vector_index_errc::invalid_argument;

        try
        {
            faiss::VectorIOReader reader;
            reader.data = std::move(buffer);
            reader.rp   = 0;

            faiss::Index *idx = faiss::read_index(&reader);
            if(!idx)
                return vector_index_errc::serialization_error;

            T *typed_idx = dynamic_cast<T *>(idx);
            if(!typed_idx)
            {
                delete idx;
                return vector_index_errc::type_mismatch;
            }

            std::unique_lock<std::shared_mutex> lock(_rw_mutex);
            _index.reset(typed_idx);
            return vector_index_errc::success;
        }
        catch(const faiss::FaissException &)
        {
            return vector_index_errc::serialization_error;
        }
        catch(...)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code save(const char *filename) const
    {
        if(!filename || filename[0] == '\0')
            return vector_index_errc::invalid_argument;

        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        try
        {
            faiss::write_index(_index.get(), filename);
            return vector_index_errc::success;
        }
        catch(...)
        {
            return vector_index_errc::io_error;
        }
    }

    std::error_code save_atomic(const char *filename) const
    {
        if(filename == nullptr || filename[0] == '\0')
            return vector_index_errc::invalid_argument;

        std::unique_ptr<faiss::Index> index_snapshot;
        {
            std::shared_lock<std::shared_mutex> lock(_rw_mutex);
            if(!_index)
                return vector_index_errc::null_index;

            try
            {
                index_snapshot.reset(faiss::clone_index(_index.get()));
            }
            catch(...)
            {
                return vector_index_errc::faiss_exception;
            }
        }

        static std::atomic<uint64_t> seq_counter{0};
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        std::string temp_file = std::string(filename) + ".tmp."
                                + std::to_string(detail::process_id()) + "."
                                + std::to_string(now) + "."
                                + std::to_string(seq_counter.fetch_add(1));

        try
        {
            faiss::write_index(index_snapshot.get(), temp_file.c_str());

            std::error_code ec;
            auto            sz = std::filesystem::file_size(temp_file, ec);
            if(ec)
            {
                std::filesystem::remove(temp_file, ec);
                return vector_index_errc::io_error;
            }
            if(sz == 0)
            {
                std::filesystem::remove(temp_file, ec);
                return vector_index_errc::file_empty;
            }

            if(!detail::atomic_rename(temp_file, filename))
            {
                if(std::filesystem::exists(temp_file))
                    std::filesystem::remove(temp_file);
                return vector_index_errc::io_error;
            }

            return vector_index_errc::success;
        }
        catch(...)
        {
            if(std::filesystem::exists(temp_file))
                std::filesystem::remove(temp_file);
            return vector_index_errc::io_error;
        }
    }

    std::error_code save_durable(const char *filename) const
    {
        if(filename == nullptr || filename[0] == '\0')
            return vector_index_errc::invalid_argument;

        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        static std::atomic<uint64_t> seq_counter{0};
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        std::string temp_file = std::string(filename) + ".tmp."
                                + std::to_string(detail::process_id()) + "."
                                + std::to_string(now) + "."
                                + std::to_string(seq_counter.fetch_add(1));

        try
        {
            faiss::write_index(_index.get(), temp_file.c_str());

            std::error_code ec;
            auto            sz = std::filesystem::file_size(temp_file, ec);
            if(ec)
            {
                std::filesystem::remove(temp_file, ec);
                return vector_index_errc::io_error;
            }
            if(sz == 0)
            {
                std::filesystem::remove(temp_file, ec);
                return vector_index_errc::file_empty;
            }

            if(!detail::sync_file(temp_file))
            {
                if(std::filesystem::exists(temp_file))
                    std::filesystem::remove(temp_file);
                return vector_index_errc::io_error;
            }

            if(!detail::atomic_rename(temp_file, filename))
            {
                if(std::filesystem::exists(temp_file))
                    std::filesystem::remove(temp_file);
                return vector_index_errc::io_error;
            }

            detail::sync_parent_dir(filename);
            return vector_index_errc::success;
        }
        catch(...)
        {
            if(std::filesystem::exists(temp_file))
            {
                std::filesystem::remove(temp_file);
            }
            return vector_index_errc::io_error;
        }
    }

    [[deprecated("Use save_atomic() for atomic file replacement.")]]
    std::error_code save_s(const char *filename) const
    {
        return save_atomic(filename);
    }

    std::error_code load(const char *filename)
    {
        if(filename == nullptr || filename[0] == '\0')
            return vector_index_errc::invalid_argument;

        std::error_code ec;

        if(!std::filesystem::exists(filename, ec))
        {
            if(ec == std::errc::permission_denied)
                return vector_index_errc::permission_denied;

            return vector_index_errc::file_not_found;
        }

        auto sz = std::filesystem::file_size(filename, ec);
        if(ec)
        {
            if(ec == std::errc::permission_denied)
                return vector_index_errc::permission_denied;

            return vector_index_errc::io_error;
        }

        if(sz == 0)
            return vector_index_errc::file_empty;

        try
        {
            faiss::Index *idx = faiss::read_index(filename);
            if(!idx)
                return vector_index_errc::serialization_error;

            T *typed_idx = dynamic_cast<T *>(idx);
            if(!typed_idx)
            {
                delete idx;
                return vector_index_errc::type_mismatch;
            }

            std::unique_lock<std::shared_mutex> lock(_rw_mutex);
            _index.reset(typed_idx);
            return vector_index_errc::success;
        }
        catch(const faiss::FaissException &)
        {
            return vector_index_errc::serialization_error;
        }
        catch(...)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    [[deprecated("Use load() instead.")]]
    std::error_code load_s(const char *filename)
    {
        return load(filename);
    }

    std::error_code get_vector_by_index(size_t index, float *vec) const
    {
        if(!vec)
            return vector_index_errc::invalid_argument;

        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(index >= static_cast<size_t>(_index->ntotal))
            return vector_index_errc::out_of_range;

        auto *id_map2 = dynamic_cast<const faiss::IndexIDMap2 *>(_index.get());
        if(id_map2 && id_map2->index)
        {
            return _reconstruct_raw(id_map2->index,
                                    static_cast<vindex_idx_t>(index),
                                    vec);
        }

        auto *id_map = dynamic_cast<const faiss::IndexIDMap *>(_index.get());
        if(id_map && id_map->index)
        {
            return _reconstruct_raw(id_map->index,
                                    static_cast<vindex_idx_t>(index),
                                    vec);
        }

        return _reconstruct(static_cast<vindex_idx_t>(index), vec);
    }

    std::error_code get_vector_by_index(size_t              index,
                                        std::vector<float> &vec) const
    {
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(index >= static_cast<size_t>(_index->ntotal))
            return vector_index_errc::out_of_range;

        vec.resize(static_cast<size_t>(_index->d));

        auto *id_map2 = dynamic_cast<const faiss::IndexIDMap2 *>(_index.get());
        if(id_map2 && id_map2->index)
        {
            return _reconstruct_raw(id_map2->index,
                                    static_cast<vindex_idx_t>(index),
                                    vec.data());
        }

        auto *id_map = dynamic_cast<const faiss::IndexIDMap *>(_index.get());
        if(id_map && id_map->index)
        {
            return _reconstruct_raw(id_map->index,
                                    static_cast<vindex_idx_t>(index),
                                    vec.data());
        }

        return _reconstruct(static_cast<vindex_idx_t>(index), vec.data());
    }

    std::error_code get_vector_by_id(vindex_idx_t id, float *vec) const
    {
        if(!vec)
            return vector_index_errc::invalid_argument;

        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        auto *id_map2 = dynamic_cast<const faiss::IndexIDMap2 *>(_index.get());
        if(id_map2)
        {
            try
            {
                id_map2->reconstruct(id, vec);
                return vector_index_errc::success;
            }
            catch(const faiss::FaissException &)
            {
                return vector_index_errc::out_of_range;
            }
            catch(...)
            {
                return vector_index_errc::faiss_exception;
            }
        }

        auto *id_map = dynamic_cast<const faiss::IndexIDMap *>(_index.get());
        if(id_map)
        {
            if(!id_map->index)
                return vector_index_errc::null_index;

            vindex_idx_t target_idx = -1;
            for(size_t i = 0; i < id_map->id_map.size(); ++i)
            {
                if(id_map->id_map[i] == id)
                {
                    target_idx = static_cast<vindex_idx_t>(i);
                    break;
                }
            }

            if(target_idx < 0)
                return vector_index_errc::out_of_range;

            return _reconstruct_raw(id_map->index, target_idx, vec);
        }

        return vector_index_errc::unsupported_operation;
    }

    std::error_code get_vector_by_id(vindex_idx_t        id,
                                     std::vector<float> &vec) const
    {
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        vec.resize(static_cast<size_t>(_index->d));

        auto *id_map2 = dynamic_cast<const faiss::IndexIDMap2 *>(_index.get());
        if(id_map2)
        {
            try
            {
                id_map2->reconstruct(id, vec.data());
                return vector_index_errc::success;
            }
            catch(const faiss::FaissException &)
            {
                return vector_index_errc::out_of_range;
            }
            catch(...)
            {
                return vector_index_errc::faiss_exception;
            }
        }

        auto *id_map = dynamic_cast<const faiss::IndexIDMap *>(_index.get());
        if(id_map)
        {
            if(!id_map->index)
                return vector_index_errc::null_index;

            vindex_idx_t target_idx = -1;
            for(size_t i = 0; i < id_map->id_map.size(); ++i)
            {
                if(id_map->id_map[i] == id)
                {
                    target_idx = static_cast<vindex_idx_t>(i);
                    break;
                }
            }

            if(target_idx < 0)
                return vector_index_errc::out_of_range;

            return _reconstruct_raw(id_map->index, target_idx, vec.data());
        }

        return vector_index_errc::unsupported_operation;
    }

    std::error_code get_all_vectors(std::vector<float> &vectors) const
    {
        std::shared_lock<std::shared_mutex> lock(_rw_mutex);
        if(!_index)
            return vector_index_errc::null_index;

        if(_index->ntotal <= 0)
        {
            vectors.clear();
            return vector_index_errc::success;
        }

        if(_index->d <= 0)
            return vector_index_errc::invalid_argument;

        const size_t ntotal      = static_cast<size_t>(_index->ntotal);
        const size_t dim         = static_cast<size_t>(_index->d);
        const size_t max_allowed = vectors.max_size();
        if(ntotal > max_allowed / dim)
            return vector_index_errc::out_of_range;

        const size_t total_elements = ntotal * dim;
        try
        {
            vectors.resize(total_elements);
        }
        catch(...)
        {
            return vector_index_errc::capacity_exceeded;
        }

        return _reconstruct_n(0, _index->ntotal, vectors.data());
    }

  private:
    std::error_code _add(vindex_count_t vectors_num,
                         const float   *vectors,
                         size_t         vectors_capacity)
    {
        if(!_index)
            return vector_index_errc::null_index;

        if(_index->d <= 0)
            return vector_index_errc::invalid_argument;

        if(!_index->is_trained)
            return vector_index_errc::not_trained;

        const size_t req_n = static_cast<size_t>(vectors_num);
        const size_t dim   = static_cast<size_t>(_index->d);

        if(req_n > (std::numeric_limits<size_t>::max)() / dim)
            return vector_index_errc::out_of_range;

        if(vectors_capacity < req_n * dim)
            return vector_index_errc::capacity_exceeded;

        try
        {
            _index->add(vectors_num, vectors);
            return vector_index_errc::success;
        }
        catch(const faiss::FaissException &)
        {
            return vector_index_errc::faiss_exception;
        }
        catch(const std::exception &)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code _reconstruct(vindex_idx_t key, float *vector) const noexcept
    {
        if(!_index || !vector)
            return vector_index_errc::invalid_argument;

        try
        {
            _index->reconstruct(key, vector);
            return vector_index_errc::success;
        }
        catch(...)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code _reconstruct_batch(vindex_count_t      n,
                                       const vindex_idx_t *keys,
                                       float *vectors) const noexcept
    {
        if(!_index || !keys || !vectors || n <= 0)
            return vector_index_errc::invalid_argument;

        try
        {
            size_t dim = _index->d;
            for(vindex_count_t i = 0; i < n; ++i)
            {
                _index->reconstruct(keys[i], vectors + i * dim);
            }
            return vector_index_errc::success;
        }
        catch(...)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code _reconstruct_n(vindex_idx_t   i0,
                                   vindex_count_t ni,
                                   float         *vectors) const noexcept
    {
        if(!_index || !vectors || ni <= 0)
            return vector_index_errc::invalid_argument;

        try
        {
            _index->reconstruct_n(i0, ni, vectors);
            return vector_index_errc::success;
        }
        catch(...)
        {
            return vector_index_errc::faiss_exception;
        }
    }

    std::error_code _reconstruct_raw(faiss::Index *raw_idx,
                                     vindex_idx_t  key,
                                     float        *vector) const noexcept
    {
        if(!raw_idx || !vector)
            return vector_index_errc::invalid_argument;

        try
        {
            raw_idx->reconstruct(key, vector);
            return vector_index_errc::success;
        }
        catch(...)
        {
            return vector_index_errc::faiss_exception;
        }
    }

  private:
    std::unique_ptr<T>        _index;
    mutable std::shared_mutex _rw_mutex;
};

} // namespace hj

namespace std
{
template <>
struct is_error_code_enum<hj::vector_index_errc> : true_type
{
};
} // namespace std

#endif // VECTOR_INDEX_HPP