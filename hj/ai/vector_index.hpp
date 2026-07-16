#ifndef VECTOR_INDEX_HPP
#define VECTOR_INDEX_HPP

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <vector>

#include <faiss/IndexFlat.h>
#include <faiss/index_io.h>
#include <faiss/impl/AuxIndexStructures.h>
#include <faiss/impl/io.h>

namespace hj
{

using vindex_flat_t             = faiss::IndexFlat;
using vindex_flat_ip_t          = faiss::IndexFlatIP;
using vindex_flat_l2_t          = faiss::IndexFlatL2;
using vindex_flat_panorama_t    = faiss::IndexFlatPanorama;
using vindex_flat_l2_panorama_t = faiss::IndexFlatL2Panorama;
using vindex_flat_ip_panorama_t = faiss::IndexFlatIPPanorama;
using vindex_flat_1d_t          = faiss::IndexFlat1D;

using vindex_dimension_t           = typename faiss::idx_t;
using vindex_idx_t                 = typename faiss::idx_t;
using vindex_count_t               = typename faiss::idx_t;
using vindex_range_search_result_t = typename faiss::RangeSearchResult;

template <typename T>
class vector_index
{
  public:
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

    ~vector_index()
    {
        if(_index)
            delete _index;
    }

    template <typename... Args>
    void build(Args &&...args)
    {
        // for example:
        //  flat_t: build(1, metric::l2);
        //  flat_ip_t: build(1);
        //  flat_l2: build(1);
        //  flat_panorama: build(1, metric::l2, 2, 512);
        //  flat_l2_panorama: build(1, 2, 512);
        //  flat_ip_panorama: build(1, 2, 512);
        //  flat_1d: build(true);
        _index = new T(std::forward<Args>(args)...);
    }

    bool add(vindex_count_t vectors_num, const float *vectors)
    {
        if(!_index)
            return false;

        _index->add(vectors_num, vectors);
        return true;
    }

    bool merge_from(const vector_index<T> &other, vindex_idx_t add_id = 0)
    {
        if(!_index)
            return false;

        _index->merge_from(*other._index, add_id);
        return true;
    }

    bool reconstruct(vindex_idx_t key, float *vector)
    {
        if(!_index)
            return false;

        _index->reconstruct(key, vector);
        return true;
    }

    bool reconstruct_batch(vindex_count_t      n,
                           const vindex_idx_t *keys,
                           float              *vectors)
    {
        if(!_index)
            return false;

        _index->reconstruct_n(n, keys, vectors);
        return true;
    }

    void search(vindex_count_t vectors_num,
                const float   *vectors,
                vindex_count_t ret_num,
                float         *distances,
                vindex_idx_t  *indexs)
    {
        if(_index)
            _index->search(vectors_num, vectors, ret_num, distances, indexs);
    }

    void range_search(vindex_count_t                vectors_num,
                      const float                  *vectors,
                      float                         radius,
                      vindex_range_search_result_t *result)
    {
        if(_index && vectors && result)
            _index->range_search(vectors_num, vectors, radius, result);
    }

    bool serialize(std::vector<uint8_t> &buffer)
    {
        if(_index == nullptr)
            return false;

        faiss::VectorIOWriter writer;
        faiss::write_index(_index, &writer);
        buffer = std::move(writer.data);
        return true;
    }

    bool deserialize(std::vector<uint8_t> &&buffer)
    {
        if(buffer.empty())
            return false;

        faiss::VectorIOReader reader;
        reader.data = std::move(buffer);
        reader.rp   = 0;

        faiss::Index *idx = faiss::read_index(&reader);
        if(idx == nullptr)
            return false;

        T *typed_idx = dynamic_cast<T *>(idx);
        if(typed_idx == nullptr)
        {
            delete idx;
            return false;
        }

        T *old_index = _index;
        _index       = typed_idx;
        if(old_index)
            delete old_index;

        return true;
    }

    bool save(const char *filename)
    {
        if(!_index)
            return false;

        faiss::write_index(_index, filename);
        return true;
    }
    bool save_s(const char *filename)
    {
        if(_index == nullptr)
            return false;

        std::string temp_file =
            std::string(filename) + ".tmp"
            + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count());
        try
        {
            faiss::write_index(_index, temp_file.c_str());

            if(!std::filesystem::exists(temp_file)
               || std::filesystem::file_size(temp_file) == 0)
                throw std::runtime_error(
                    "Failed to write index to temporary file.");

            auto *loaded = faiss::read_index(temp_file.c_str());
            if(loaded == nullptr || loaded->ntotal != _index->ntotal)
                throw std::runtime_error(
                    "Failed to verify index after writing to temporary file.");
            delete loaded;

            std::filesystem::rename(temp_file, filename);
            return true;
        }
        catch(const faiss::FaissException &e)
        {
            std::cerr << e.what() << std::endl;
        }
        catch(const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
        catch(...)
        {
            std::cerr << "Unknown error occurred while saving index."
                      << std::endl;
        }
        if(std::filesystem::exists(temp_file))
            std::filesystem::remove(temp_file);

        return false;
    }

    bool load(const char *filename)
    {
        if(_index)
            delete _index;

        auto idx = faiss::read_index(filename);
        if(idx == nullptr)
            return false;

        _index = static_cast<T *>(idx);
        return true;
    }
    bool load_s(const char *filename)
    {
        if(filename == nullptr || filename[0] == '\0')
        {
            std::cerr << "Error: Invalid filename (null or empty)" << std::endl;
            return false;
        }

        if(!std::filesystem::exists(filename))
        {
            std::cerr << "Error: File does not exist: " << filename
                      << std::endl;
            return false;
        }

        try
        {
            if(std::filesystem::file_size(filename) == 0)
            {
                std::cerr << "Error: File is empty: " << filename << std::endl;
                return false;
            }
        }
        catch(const std::filesystem::filesystem_error &e)
        {
            std::cerr << "Error: Failed to get file size: " << e.what()
                      << std::endl;
            return false;
        }

        faiss::Index *idx = nullptr;
        try
        {
            idx = faiss::read_index(filename);
            if(idx == nullptr)
            {
                std::cerr << "Error: Failed to read index from file: "
                          << filename << std::endl;
                return false;
            }

            T *typed_idx = dynamic_cast<T *>(idx);
            if(typed_idx == nullptr)
            {
                std::cerr << "Error: Index type mismatch. Expected type: "
                          << typeid(T).name() << std::endl;
                delete idx;
                return false;
            }

            T *old_index = _index;
            _index       = typed_idx;
            if(old_index)
                delete old_index;

            return true;
        }
        catch(const faiss::FaissException &e)
        {
            std::cerr << "FAISS error while loading index: " << e.what()
                      << std::endl;
        }
        catch(const std::filesystem::filesystem_error &e)
        {
            std::cerr << "Filesystem error while loading index: " << e.what()
                      << std::endl;
        }
        catch(const std::exception &e)
        {
            std::cerr << "Standard error while loading index: " << e.what()
                      << std::endl;
        }
        catch(...)
        {
            std::cerr << "Unknown error occurred while loading index."
                      << std::endl;
        }

        if(idx)
            delete idx;

        return false;
    }

    bool get_vector(int64_t idx, float *vec) const
    {
        if(!_index || idx < 0
           || static_cast<vindex_idx_t>(idx) >= _index->ntotal
           || vec == nullptr)
            return false;

        _index->reconstruct(static_cast<vindex_idx_t>(idx), vec);
        return true;
    }

    bool get_vector(int64_t idx, std::vector<float> &vec) const
    {
        if(!_index || idx < 0
           || static_cast<vindex_idx_t>(idx) >= _index->ntotal)
            return false;

        int dim = _index->d;
        vec.resize(dim);
        _index->reconstruct(static_cast<vindex_idx_t>(idx), vec.data());
        return true;
    }

    bool get_all_vectors(std::vector<float> &vectors) const
    {
        if(!_index || _index->ntotal == 0)
            return false;

        int dim = _index->d;
        int n   = _index->ntotal;
        vectors.resize(n * dim);

        _index->reconstruct_n(0, n, vectors.data());
        return true;
    }

    void reset()
    {
        if(_index)
            _index->reset();
    }

    vindex_count_t total() const
    {
        if(_index)
            return _index->ntotal;

        return 0;
    }

  private:
    T *_index;
};

} // namespace hj

#endif // VECTOR_INDEX_HPP