#ifndef LLAMA_HPP
#define LLAMA_HPP

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "llama.h"

namespace hj::llama
{

enum class error_code
{
    success = 0,
    file_not_found,
    permission_denied,
    invalid_path,
    invalid_model,
    unsupported_model,
    out_of_memory,
    file_write_failure,
    backend_error,
    invalid_argument,
    buffer_overflow,
    invalid_state,
    context_init_failed,
    sampler_init_failed
};

class llama_err_category : public std::error_category
{
  public:
    const char *name() const noexcept override { return "hj::llama"; }

    std::string message(int ev) const override
    {
        switch(static_cast<error_code>(ev))
        {
            case error_code::success:
                return "Success";
            case error_code::file_not_found:
                return "Model file not found";
            case error_code::permission_denied:
                return "Permission denied reading model file";
            case error_code::invalid_path:
                return "Invalid file path or not a regular file";
            case error_code::invalid_model:
                return "Invalid or corrupted GGUF format";
            case error_code::unsupported_model:
                return "Unsupported model architecture or version";
            case error_code::out_of_memory:
                return "Insufficient memory or GPU VRAM";
            case error_code::file_write_failure:
                return "Failed to save model to disk";
            case error_code::backend_error:
                return "Internal backend failure during model loading";
            case error_code::invalid_argument:
                return "Invalid argument supplied to function";
            case error_code::buffer_overflow:
                return "Batch capacity exceeded";
            case error_code::invalid_state:
                return "Operation attempted on uninitialized resource";
            case error_code::context_init_failed:
                return "Failed to initialize context from model";
            case error_code::sampler_init_failed:
                return "Failed to initialize sampler chain";
            default:
                return "Unknown model error";
        }
    }
};

inline const std::error_category &llama_err_category_instance()
{
    static llama_err_category instance;
    return instance;
}

inline std::error_code make_error_code(error_code e) noexcept
{
    return std::error_code(static_cast<int>(e),
                           hj::llama::llama_err_category_instance());
}

} // namespace hj::llama

template <>
struct std::is_error_code_enum<hj::llama::error_code> : std::true_type
{
};


namespace hj
{

namespace llama
{
using model_params_t         = llama_model_params;
using context_params_t       = llama_context_params;
using sampler_chain_params_t = llama_sampler_chain_params;

using vocab_t        = llama_vocab;
using memory_t       = llama_memory_t;
using threadpool_t   = ggml_threadpool_t;
using gguf_ctx_t     = gguf_context;
using adapter_lora_t = llama_adapter_lora;
using logit_bias_t   = llama_logit_bias;

using pos_t    = llama_pos;
using token_t  = llama_token;
using seq_id_t = llama_seq_id;

using set_tensor_data_fn = llama_model_set_tensor_data_t;

static constexpr std::size_t model_desc_sz = 2048;

namespace detail
{

class backend_mgr
{
  public:
    static backend_mgr &instance()
    {
        static backend_mgr mgr;
        return mgr;
    }

    void retain()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_ref_count == 0)
        {
            llama_backend_init();
        }
        ++_ref_count;
    }

    void release() noexcept
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_ref_count > 0)
        {
            --_ref_count;
            if(_ref_count == 0)
            {
                llama_backend_free();
            }
        }
    }

  private:
    backend_mgr() = default;
    ~backend_mgr()
    {
        if(_ref_count > 0)
        {
            llama_backend_free();
            _ref_count = 0;
        }
    }

    backend_mgr(const backend_mgr &)            = delete;
    backend_mgr &operator=(const backend_mgr &) = delete;

    std::mutex  _mutex;
    std::size_t _ref_count{0};
};

class backend_guard
{
  public:
    backend_guard() { backend_mgr::instance().retain(); }
    ~backend_guard() { backend_mgr::instance().release(); }

    backend_guard(const backend_guard &)            = delete;
    backend_guard &operator=(const backend_guard &) = delete;

    backend_guard(backend_guard &&) noexcept            = default;
    backend_guard &operator=(backend_guard &&) noexcept = default;
};

// Custom Deleters for C Pointers (RAII)
struct llama_model_deleter
{
    void operator()(llama_model *ptr) const
    {
        if(ptr)
            llama_model_free(ptr);
    }
};

struct llama_context_deleter
{
    void operator()(llama_context *ptr) const
    {
        if(ptr)
            llama_free(ptr);
    }
};

struct llama_adapter_lora_deleter
{
    void operator()(llama_adapter_lora *ptr) const
    {
        if(ptr)
            llama_adapter_lora_free(ptr);
    }
};

struct llama_sampler_deleter
{
    void operator()(llama_sampler *ptr) const
    {
        if(ptr)
            llama_sampler_free(ptr);
    }
};

static std::error_code validate_file_path(const std::filesystem::path &filename)
{
    std::error_code ec;
    bool            exists = std::filesystem::exists(filename, ec);
    if(ec || !exists)
        return error_code::file_not_found;

    bool is_reg = std::filesystem::is_regular_file(filename, ec);
    if(ec || !is_reg)
        return error_code::invalid_path;

    return error_code::success;
}

} // namespace detail

// 导出 backend_guard 到 hj::llama 命名空间
using backend_guard = detail::backend_guard;

static inline bool supports_mmap()
{
    return llama_supports_mmap();
}

static inline bool supports_mlock()
{
    return llama_supports_mlock();
}

static inline bool supports_gpu_offload()
{
    return llama_supports_gpu_offload();
}

static inline bool supports_rpc()
{
    return llama_supports_rpc();
}

/**
 * @brief Thread Safety Contract for `hj::llama::batch`:
 *
 * - All Operations (set_tokens(), set_logits(), clear()): [NOT THREAD-SAFE]
 *   `batch` is a lightweight memory buffer container for batch evaluation.
 *   Pass by value/move or use thread-local instances per inference step.
 */
class batch
{
  public:
    batch(int32_t n_tokens, int32_t embd, int32_t n_seq_max)
    {
        if(n_tokens <= 0 || embd < 0 || n_seq_max <= 0)
        {
            throw std::invalid_argument(
                "hj::llama::batch: Invalid initialization arguments");
        }

        _batch = llama_batch_init(n_tokens, embd, n_seq_max);
        bool alloc_failed =
            (embd > 0) ? (_batch.embd == nullptr) : (_batch.token == nullptr);
        if(alloc_failed)
        {
            throw std::bad_alloc{};
        }

        _capacity = n_tokens;
    }

    explicit batch(const std::vector<token_t> &tokens)
        : batch(static_cast<int32_t>(tokens.size()), 0, 1)
    {
        if(tokens.empty())
        {
            free_batch();
            throw std::invalid_argument(
                "hj::llama::batch: Token vector cannot be empty");
        }

        auto err =
            set_tokens(tokens.data(), static_cast<int32_t>(tokens.size()), 0);
        if(err)
        {
            free_batch();
            throw std::invalid_argument(err.message());
        }
    }

    batch(const token_t *tokens, int32_t n_tokens)
        : batch(n_tokens, 0, 1)
    {
        if(!tokens)
        {
            free_batch();
            throw std::invalid_argument(
                "hj::llama::batch: Invalid token pointer");
        }

        auto err = set_tokens(tokens, n_tokens, 0);
        if(err)
        {
            free_batch();
            throw std::invalid_argument(err.message());
        }
    }

    ~batch() { free_batch(); }

    batch(const batch &)            = delete;
    batch &operator=(const batch &) = delete;

    batch(batch &&other) noexcept
        : _capacity(other._capacity)
        , _batch(other._batch)
    {
        std::memset(&other._batch, 0, sizeof(llama_batch));
        other._capacity = 0;
    }

    batch &operator=(batch &&other) noexcept
    {
        if(this != &other)
        {
            free_batch();

            _batch    = other._batch;
            _capacity = other._capacity;

            std::memset(&other._batch, 0, sizeof(llama_batch));
            other._capacity = 0;
        }
        return *this;
    }

    const llama_batch &get() const noexcept { return _batch; }
    const llama_batch *data() const noexcept { return &_batch; }

    int32_t size() const noexcept { return _batch.n_tokens; }
    int32_t capacity() const noexcept { return _capacity; }
    bool    empty() const noexcept { return _batch.n_tokens == 0; }

    void clear() noexcept { _batch.n_tokens = 0; }

    void set_logits(int32_t idx, bool value)
    {
        if(idx >= 0 && idx < _batch.n_tokens)
            _batch.logits[idx] = value;
    }

    std::error_code
    set_tokens(const token_t *tokens, int32_t n_tokens, int32_t start_pos = 0)
    {
        if(n_tokens == 0)
        {
            _batch.n_tokens = 0;
            return make_error_code(error_code::success);
        }

        if(!tokens || n_tokens < 0)
        {
            _batch.n_tokens = 0;
            return make_error_code(error_code::invalid_argument);
        }

        if(n_tokens > _capacity)
        {
            return make_error_code(error_code::buffer_overflow);
        }

        for(int32_t i = 0; i < n_tokens; ++i)
        {
            _batch.token[i]     = tokens[i];
            _batch.pos[i]       = start_pos + i;
            _batch.n_seq_id[i]  = 1;
            _batch.seq_id[i][0] = 0;
            _batch.logits[i]    = false;
        }
        _batch.n_tokens = n_tokens;

        return make_error_code(error_code::success);
    }

    std::error_code set_tokens(const std::vector<token_t> &tokens,
                               int32_t                     start_pos = 0)
    {
        return set_tokens(tokens.data(),
                          static_cast<int32_t>(tokens.size()),
                          start_pos);
    }

  private:
    void free_batch()
    {
        if(_batch.token != nullptr || _batch.embd != nullptr)
        {
            llama_batch_free(_batch);
            std::memset(&_batch, 0, sizeof(llama_batch));
        }
    }

    int32_t     _capacity = 0;
    llama_batch _batch{};
};

class memory_view
{
  public:
    constexpr memory_view() noexcept = default;

    explicit constexpr memory_view(llama_memory_t mem) noexcept
        : _mem(mem)
    {
    }

    constexpr llama_memory_t data() const noexcept { return _mem; }

    explicit constexpr operator bool() const noexcept
    {
        return _mem != nullptr;
    }

    constexpr bool operator==(std::nullptr_t) const noexcept
    {
        return _mem == nullptr;
    }
    constexpr bool operator!=(std::nullptr_t) const noexcept
    {
        return _mem != nullptr;
    }

    void clear(bool data) const
    {
        if(_mem)
            llama_memory_clear(_mem, data);
    }

    void seq_rm(seq_id_t seq_id, pos_t p0, pos_t p1) const
    {
        if(_mem)
            llama_memory_seq_rm(_mem, seq_id, p0, p1);
    }

    void
    seq_cp(seq_id_t seq_id_src, seq_id_t seq_id_dst, pos_t p0, pos_t p1) const
    {
        if(_mem)
            llama_memory_seq_cp(_mem, seq_id_src, seq_id_dst, p0, p1);
    }

    void seq_keep(seq_id_t seq_id) const
    {
        if(_mem)
            llama_memory_seq_keep(_mem, seq_id);
    }

    void seq_add(seq_id_t seq_id, pos_t p0, pos_t p1, pos_t delta) const
    {
        if(_mem)
            llama_memory_seq_add(_mem, seq_id, p0, p1, delta);
    }

    void seq_div(seq_id_t seq_id, pos_t p0, pos_t p1, int d) const
    {
        if(_mem)
            llama_memory_seq_div(_mem, seq_id, p0, p1, d);
    }

    void seq_pos_min(seq_id_t seq_id) const
    {
        if(_mem)
            llama_memory_seq_pos_min(_mem, seq_id);
    }

    void seq_pos_max(seq_id_t seq_id) const
    {
        if(_mem)
            llama_memory_seq_pos_max(_mem, seq_id);
    }

    bool can_shift() const
    {
        return _mem ? llama_memory_can_shift(_mem) : false;
    }

  private:
    llama_memory_t _mem = nullptr;
};

using memory = memory_view;

/**
 * @brief Thread Safety Contract for `hj::llama::model`:
 *
 * - Read-only Metadata & Vocab Queries: [THREAD-SAFE]
 *   Functions like n_embd(), n_vocab(), size(), desc(), token_is_eog(), etc., 
 *   as well as const metadata queries, are thread-safe and can be called concurrently 
 *   by multiple threads on the same `model` instance.
 *
 * - Tokenization (tokenize()): [THREAD-SAFE]
 *   Underlying `llama_tokenize` calls with `llama_vocab` read from immutable vocabulary state.
 *   Safe for concurrent invocation across threads.
 *
 * - State Modifying / Lifecycle Operations (load(), reset(), move/copy): [NOT THREAD-SAFE]
 *   Mutating operations require external synchronization (e.g., exclusive access / std::unique_lock).
 */
class model
{
  public:
    model() = default;

    explicit model(llama_model *m)
        : _backend_guard(std::make_shared<detail::backend_guard>())
        , _model(m)
    {
    }

    explicit model(const std::filesystem::path &filename,
                   model_params_t params = llama_model_default_params())
    {
        load(filename, params);
    }

    model(FILE *file, model_params_t params = llama_model_default_params())
    {
        load(file, params);
    }

    model(const char   **paths,
          size_t         n_paths,
          model_params_t params = llama_model_default_params())
    {
        load(paths, n_paths, params);
    }

    model(gguf_ctx_t        *metadata,
          set_tensor_data_fn set_tensor_data,
          void              *set_tensor_data_ud,
          model_params_t     params)
        : _backend_guard(std::make_shared<detail::backend_guard>())
        , _model(llama_model_init_from_user(
              metadata, set_tensor_data, set_tensor_data_ud, params))
    {
    }

    ~model() = default;

    model(const model &)            = delete;
    model &operator=(const model &) = delete;

    model(model &&other) noexcept            = default;
    model &operator=(model &&other) noexcept = default;

    static model_params_t default_params()
    {
        return llama_model_default_params();
    }

    llama_model *data() const { return _model.get(); }

    std::error_code load(const std::filesystem::path &filename,
                         model_params_t params = llama_model_default_params())
    {
        auto err = detail::validate_file_path(filename);
        if(err)
            return err;

        auto guard = std::make_shared<detail::backend_guard>();
        auto loaded =
            llama_model_load_from_file(filename.string().c_str(), params);

        if(!loaded)
            return make_error_code(error_code::invalid_model);

        _backend_guard = std::move(guard);
        _model.reset(loaded);
        return {};
    }

    std::error_code load(FILE          *file,
                         model_params_t params = llama_model_default_params())
    {
        if(file == nullptr)
            return make_error_code(error_code::invalid_path);

        auto guard  = std::make_shared<detail::backend_guard>();
        auto loaded = llama_model_load_from_file_ptr(file, params);
        if(!loaded)
            return make_error_code(error_code::invalid_model);

        _backend_guard = std::move(guard);
        _model.reset(loaded);
        return {};
    }

    std::error_code load(const char   **paths,
                         size_t         n_paths,
                         model_params_t params = llama_model_default_params())
    {
        for(size_t i = 0; i < n_paths; ++i)
        {
            auto err = detail::validate_file_path(paths[i]);
            if(err)
                return err;
        }

        auto guard  = std::make_shared<detail::backend_guard>();
        auto loaded = llama_model_load_from_splits(paths, n_paths, params);
        if(!loaded)
            return make_error_code(error_code::invalid_model);

        _backend_guard = std::move(guard);
        _model.reset(loaded);
        return {};
    }

    /**
     * @brief Saves the underlying model to disk safely and atomically.
     * 
     * @details Standard C API llama_model_save_to_file() does not return a status code.
     *          To prevent corruption of existing files and ensure cross-platform write capability,
     *          this function performs an empirical write-test and atomic rename strategy:
     *          1. Ensures parent path valid & target is not a directory.
     *          2. Generates a unique temporary filename.
     *          3. Empirical test: Attempts to open the temp file with write permissions 
     *             to directly query system OS/ACL write access (avoiding unreliable std::filesystem::perms).
     *          4. Invokes llama_model_save_to_file() on the temporary path.
     *          5. Verifies temporary file creation and non-zero byte size.
     *          6. Atomically renames temporary file to destination path, overwriting standard target.
     * 
     * @param filename Target file path.
     * @return std::error_code Status of the save operation.
     */
    std::error_code save(const std::filesystem::path &filename) const
    {
        if(!_model)
            return make_error_code(error_code::invalid_model);

        if(filename.empty())
            return make_error_code(error_code::invalid_path);

        std::error_code ec;

        if(std::filesystem::is_directory(filename, ec))
            return make_error_code(error_code::invalid_path);

        const auto parent_path = filename.parent_path();
        const auto target_dir =
            parent_path.empty() ? std::filesystem::current_path() : parent_path;

        if(!std::filesystem::exists(target_dir, ec) || ec)
            return make_error_code(error_code::file_not_found);

        auto tmp_filename = filename;
        tmp_filename += ".tmp." + std::to_string(std::rand());

        struct tmp_cleaner
        {
            std::filesystem::path path;
            ~tmp_cleaner()
            {
                if(!path.empty())
                {
                    std::error_code ignore_ec;
                    std::filesystem::remove(path, ignore_ec);
                }
            }
        } cleaner{tmp_filename};

        {
            FILE *test_fp = std::fopen(tmp_filename.string().c_str(), "wb");
            if(!test_fp)
            {
                if(errno == EACCES || errno == EPERM)
                {
                    return make_error_code(error_code::permission_denied);
                }
                return make_error_code(error_code::file_write_failure);
            }
            std::fclose(test_fp);
            std::filesystem::remove(tmp_filename, ec);
        }

        llama_model_save_to_file(_model.get(), tmp_filename.string().c_str());

        if(!std::filesystem::is_regular_file(tmp_filename, ec) || ec)
            return make_error_code(error_code::file_write_failure);

        auto file_size = std::filesystem::file_size(tmp_filename, ec);
        if(ec || file_size == 0)
            return make_error_code(error_code::file_write_failure);

        std::filesystem::rename(tmp_filename, filename, ec);
        if(ec)
        {
            if(ec.value() == static_cast<int>(std::errc::permission_denied))
                return make_error_code(error_code::permission_denied);
            return make_error_code(error_code::file_write_failure);
        }

        cleaner.path.clear();

        return make_error_code(error_code::success);
    }

    std::vector<token_t> tokenize(const std::string &prompt,
                                  bool               add_special,
                                  bool               parse_special,
                                  std::error_code   &err) const
    {
        if(!_model)
        {
            err = make_error_code(error_code::invalid_model);
            return {};
        }

        if(prompt.empty())
        {
            err = make_error_code(error_code::success);
            return {};
        }

        if(prompt.size()
           > static_cast<std::size_t>(std::numeric_limits<int32_t>::max()))
        {
            err = make_error_code(error_code::buffer_overflow);
            return {};
        }

        auto vocab = llama_model_get_vocab(_model.get());
        if(!vocab)
        {
            err = make_error_code(error_code::invalid_state);
            return {};
        }

        const auto prompt_len = static_cast<int32_t>(prompt.length());
        auto       n_tokens   = llama_tokenize(vocab,
                                               prompt.c_str(),
                                               prompt_len,
                                               nullptr,
                                               0,
                                               add_special,
                                               parse_special);

        if(n_tokens == 0)
        {
            err = make_error_code(error_code::success);
            return {};
        }

        const int32_t n_tokens_needed = (n_tokens < 0) ? -n_tokens : n_tokens;
        std::vector<token_t> tokens(n_tokens_needed);
        n_tokens = llama_tokenize(vocab,
                                  prompt.c_str(),
                                  prompt_len,
                                  tokens.data(),
                                  static_cast<int32_t>(tokens.size()),
                                  add_special,
                                  parse_special);
        if(n_tokens < 0)
        {
            err = make_error_code(error_code::buffer_overflow);
            return {};
        }

        tokens.resize(n_tokens);
        err = make_error_code(error_code::success);
        return tokens;
    }

    uint64_t size() const
    {
        return _model ? llama_model_size(_model.get()) : 0;
    }

    std::string desc() const
    {
        if(!_model)
            return {};

        char buf[model_desc_sz] = {0};
        auto sz = llama_model_desc(_model.get(), buf, sizeof(buf));
        return std::string(buf, sz);
    }

    int32_t n_ctx_train() const
    {
        return _model ? llama_model_n_ctx_train(_model.get()) : 0;
    }
    int32_t n_embd() const
    {
        return _model ? llama_model_n_embd(_model.get()) : 0;
    }
    int32_t n_embd_inp() const
    {
        return _model ? llama_model_n_embd_inp(_model.get()) : 0;
    }
    int32_t n_embd_out() const
    {
        return _model ? llama_model_n_embd_out(_model.get()) : 0;
    }
    int32_t n_layer() const
    {
        return _model ? llama_model_n_layer(_model.get()) : 0;
    }
    int32_t n_head() const
    {
        return _model ? llama_model_n_head(_model.get()) : 0;
    }
    int32_t n_head_kv() const
    {
        return _model ? llama_model_n_head_kv(_model.get()) : 0;
    }
    int32_t n_swa() const
    {
        return _model ? llama_model_n_swa(_model.get()) : 0;
    }

    const vocab_t *get_vocab() const
    {
        return _model ? llama_model_get_vocab(_model.get()) : nullptr;
    }

    int n_vocab() const
    {
        auto vocab = get_vocab();
        return vocab ? llama_n_vocab(vocab) : 0;
    }

    bool token_is_eog(token_t token) const
    {
        auto vocab = get_vocab();
        return vocab ? llama_token_is_eog(vocab, token) : false;
    }

    int32_t token_to_piece(token_t token,
                           char   *buf,
                           int32_t length,
                           int32_t lstrip,
                           bool    special) const
    {
        auto vocab = get_vocab();
        return vocab ? llama_token_to_piece(vocab,
                                            token,
                                            buf,
                                            length,
                                            lstrip,
                                            special)
                     : -1;
    }

    llama_rope_type rope_type() const
    {
        return _model ? llama_model_rope_type(_model.get())
                      : LLAMA_ROPE_TYPE_NONE;
    }

    float rope_freq_scale_train() const
    {
        return _model ? llama_model_rope_freq_scale_train(_model.get()) : 0.0f;
    }

    uint32_t n_cls_out() const
    {
        return _model ? llama_model_n_cls_out(_model.get()) : 0;
    }

    const char *cls_label(uint32_t i) const
    {
        return _model ? llama_model_cls_label(_model.get(), i) : nullptr;
    }

    int32_t meta_val_str(const char *key, char *buf, size_t buf_size) const
    {
        return _model
                   ? llama_model_meta_val_str(_model.get(), key, buf, buf_size)
                   : -1;
    }

    int32_t meta_count() const
    {
        return _model ? llama_model_meta_count(_model.get()) : -1;
    }

    int32_t meta_key_by_index(int32_t i, char *buf, size_t buf_size) const
    {
        return _model ? llama_model_meta_key_by_index(_model.get(),
                                                      i,
                                                      buf,
                                                      buf_size)
                      : -1;
    }

    std::string chat_template(const char *name) const
    {
        if(!_model)
            return {};
        const char *tmpl = llama_model_chat_template(_model.get(), name);
        return tmpl ? std::string(tmpl) : std::string();
    }

    uint64_t n_params() const
    {
        return _model ? llama_model_n_params(_model.get()) : 0;
    }

    bool has_encoder() const
    {
        return _model ? llama_model_has_encoder(_model.get()) : false;
    }
    bool has_decoder() const
    {
        return _model ? llama_model_has_decoder(_model.get()) : false;
    }
    bool is_recurrent() const
    {
        return _model ? llama_model_is_recurrent(_model.get()) : false;
    }
    bool is_hybrid() const
    {
        return _model ? llama_model_is_hybrid(_model.get()) : false;
    }
    bool is_diffusion() const
    {
        return _model ? llama_model_is_diffusion(_model.get()) : false;
    }

    token_t decoder_start_token() const
    {
        return _model ? llama_model_decoder_start_token(_model.get()) : -1;
    }

  private:
    std::shared_ptr<detail::backend_guard> _backend_guard{nullptr};
    std::unique_ptr<llama_model, detail::llama_model_deleter> _model{nullptr};
};

/**
 * @brief Thread Safety Contract for `hj::llama::adapter_lora`:
 *
 * - Metadata Queries & Data Pointer Access: [THREAD-SAFE]
 *   Once initialized, LoRA adapter weights are read-only and thread-safe.
 * 
 * - Initialization / Destruction (init(), reset()): [NOT THREAD-SAFE]
 *   Requires external synchronization.
 */
class adapter_lora
{
  public:
    adapter_lora() = default;

    adapter_lora(std::shared_ptr<const model> m, const char *path_lora)
    {
        if(auto err = init(std::move(m), path_lora); err)
        {
            throw std::runtime_error("hj::llama::adapter_lora: "
                                     + err.message());
        }
    }

    adapter_lora(const model &m, const char *path_lora)
        : adapter_lora(std::shared_ptr<const model>(&m, [](const model *) {}),
                       path_lora)
    {
    }

    ~adapter_lora() = default;

    adapter_lora(const adapter_lora &)            = delete;
    adapter_lora &operator=(const adapter_lora &) = delete;

    adapter_lora(adapter_lora &&) noexcept            = default;
    adapter_lora &operator=(adapter_lora &&) noexcept = default;

    std::error_code init(std::shared_ptr<const model> m, const char *path_lora)
    {
        if(!m || !m->data() || !path_lora)
        {
            return make_error_code(error_code::invalid_argument);
        }

        adapter_lora_t *raw_adapter =
            llama_adapter_lora_init(m->data(), path_lora);
        if(!raw_adapter)
        {
            return make_error_code(error_code::backend_error);
        }

        _adapter.reset(raw_adapter);
        _model = std::move(m);
        return make_error_code(error_code::success);
    }

    adapter_lora_t *data() const { return _adapter.get(); }

    void reset() noexcept
    {
        _adapter.reset();
        _model.reset();
    }

    int32_t meta_val_str(const char *key, char *buf, size_t buf_size) const
    {
        return _adapter ? llama_adapter_meta_val_str(_adapter.get(),
                                                     key,
                                                     buf,
                                                     buf_size)
                        : -1;
    }

    int32_t meta_count() const
    {
        return _adapter ? llama_adapter_meta_count(_adapter.get()) : -1;
    }

    int32_t meta_key_by_index(int32_t i, char *buf, size_t buf_size) const
    {
        return _adapter ? llama_adapter_meta_key_by_index(_adapter.get(),
                                                          i,
                                                          buf,
                                                          buf_size)
                        : -1;
    }

    uint64_t meta_val_str_by_index(int32_t i, char *buf, size_t buf_size) const
    {
        return _adapter ? llama_adapter_meta_val_str_by_index(_adapter.get(),
                                                              i,
                                                              buf,
                                                              buf_size)
                        : 0;
    }

    uint64_t get_alora_n_invocation_tokens() const
    {
        return _adapter
                   ? llama_adapter_get_alora_n_invocation_tokens(_adapter.get())
                   : 0;
    }

    const token_t *get_alora_invocation_tokens() const
    {
        return _adapter
                   ? llama_adapter_get_alora_invocation_tokens(_adapter.get())
                   : nullptr;
    }

  private:
    std::shared_ptr<const model> _model{nullptr};
    std::unique_ptr<adapter_lora_t, detail::llama_adapter_lora_deleter>
        _adapter{nullptr};
};

/**
 * @brief Thread Safety Contract for `hj::llama::context`:
 *
 * - All Operations (decode(), get_logits_ith(), state_set_data(), get_memory(), etc.): [NOT THREAD-SAFE]
 *   A single `context` instance holds dynamic KV cache state and execution graphs.
 *   It MUST NOT be accessed concurrently by multiple threads.
 *
 * - Concurrency Pattern:
 *   1. [Multi-Context Multi-Thread]: Load ONE `model` once, create ONE `context` per thread/session.
 *   2. [Single Context Shared]: If shared across HTTP Workers, external synchronization (e.g., std::mutex)
 *      or a Task Queue / Context Pool MUST be used.
 */
class context
{
  public:
    context() noexcept = default;

    context(std::shared_ptr<const model> m,
            context_params_t             params = default_params())
    {
        if(auto err = init(std::move(m), params); err)
        {
            throw std::runtime_error("hj::llama::context: " + err.message());
        }
    }

    context(const model &m, context_params_t params = default_params())
        : context(std::shared_ptr<const model>(&m, [](const model *) {}),
                  params)
    {
    }

    context(const context &)            = delete;
    context &operator=(const context &) = delete;

    context(context &&) noexcept            = default;
    context &operator=(context &&) noexcept = default;

    ~context() = default;

    static context_params_t default_params()
    {
        return llama_context_default_params();
    }

    llama_context *data() const noexcept { return _ctx.get(); }
    explicit       operator bool() const noexcept { return _ctx != nullptr; }

    void reset() noexcept
    {
        _ctx.reset();
        _model.reset();
    }

    /// @brief Re-initializes the context with a shared model pointer and params.
    /// @details Guarantees Strong Exception Safety: If initialization fails,
    ///          the old context state is completely untouched.
    /// @return std::error_code Indicating success or the failure reason.
    std::error_code init(std::shared_ptr<const model> m,
                         context_params_t             params)
    {
        if(!m || !m->data())
        {
            return make_error_code(error_code::invalid_argument);
        }

        // Attempt new allocation before invalidating current resource
        llama_context *new_raw = llama_init_from_model(m->data(), params);
        if(!new_raw)
        {
            return make_error_code(error_code::backend_error);
        }

        _ctx.reset(new_raw);
        _model = std::move(m);
        return make_error_code(error_code::success);
    }

    std::error_code init(const model &m, context_params_t params)
    {
        return init(std::shared_ptr<const model>(&m, [](const model *) {}),
                    params);
    }

    std::error_code init(const model *m, context_params_t params)
    {
        if(!m)
        {
            return make_error_code(error_code::invalid_argument);
        }
        return init(*m, params);
    }

    memory_view get_memory() const noexcept
    {
        return _ctx ? memory_view(llama_get_memory(_ctx.get())) : memory_view();
    }

    size_t state_get_size() const
    {
        return _ctx ? llama_state_get_size(_ctx.get()) : 0;
    }

    float *get_embeddings() const
    {
        return _ctx ? llama_get_embeddings(_ctx.get()) : nullptr;
    }

    float *get_embeddings_ith(int32_t i) const
    {
        return _ctx ? llama_get_embeddings_ith(_ctx.get(), i) : nullptr;
    }

    float *get_embeddings_seq(seq_id_t seq_id) const
    {
        return _ctx ? llama_get_embeddings_seq(_ctx.get(), seq_id) : nullptr;
    }

    std::error_code attach_threadpool(threadpool_t threadpool,
                                      threadpool_t threadpool_batch)
    {
        if(!_ctx)
            return make_error_code(error_code::invalid_argument);

        llama_attach_threadpool(_ctx.get(), threadpool, threadpool_batch);
        return make_error_code(error_code::success);
    }

    std::error_code detach_threadpool()
    {
        if(!_ctx)
            return make_error_code(error_code::invalid_argument);

        llama_detach_threadpool(_ctx.get());
        return make_error_code(error_code::success);
    }

    uint32_t n_ctx() const { return _ctx ? llama_n_ctx(_ctx.get()) : 0; }
    uint32_t n_ctx_seq() const
    {
        return _ctx ? llama_n_ctx_seq(_ctx.get()) : 0;
    }
    uint32_t n_batch() const { return _ctx ? llama_n_batch(_ctx.get()) : 0; }
    uint32_t n_ubatch() const { return _ctx ? llama_n_ubatch(_ctx.get()) : 0; }
    uint32_t n_seq_max() const
    {
        return _ctx ? llama_n_seq_max(_ctx.get()) : 0;
    }

    int32_t set_adapter_lora(const std::vector<adapter_lora> &loras,
                             float                           *scales)
    {
        if(!_ctx)
            return -1;

        std::vector<adapter_lora_t *> lora_ptrs;
        lora_ptrs.reserve(loras.size());
        for(const auto &lora : loras)
        {
            if(!lora.data())
                return -1;
            lora_ptrs.push_back(lora.data());
        }

        return llama_set_adapters_lora(_ctx.get(),
                                       lora_ptrs.data(),
                                       lora_ptrs.size(),
                                       scales);
    }

    int32_t set_adapter_cvec(const float *data,
                             size_t       len,
                             int32_t      n_embd,
                             int32_t      il_start,
                             int32_t      il_end)
    {
        if(!_ctx)
            return -1;
        return llama_set_adapter_cvec(_ctx.get(),
                                      data,
                                      len,
                                      n_embd,
                                      il_start,
                                      il_end);
    }

    size_t state_set_data(const uint8_t *src, size_t size)
    {
        if(!_ctx)
            return 0;
        return llama_state_set_data(_ctx.get(), src, size);
    }

    std::error_code decode(llama_batch b)
    {
        if(!_ctx)
            return make_error_code(error_code::invalid_state);

        int res = llama_decode(_ctx.get(), b);
        if(res != 0)
            return make_error_code(error_code::backend_error);

        return make_error_code(error_code::success);
    }

    std::error_code decode(const batch &b)
    {
        if(!_ctx)
            return make_error_code(error_code::invalid_state);

        int res = llama_decode(_ctx.get(), b.get());
        if(res != 0)
            return make_error_code(error_code::backend_error);

        return make_error_code(error_code::success);
    }

    const float *get_logits_ith(int32_t i) const
    {
        if(!_ctx)
            return nullptr;
        return llama_get_logits_ith(_ctx.get(), i);
    }

  private:
    std::shared_ptr<const model> _model{nullptr};
    std::unique_ptr<llama_context, detail::llama_context_deleter> _ctx{nullptr};
};

struct sampler_options
{
    // Penalties
    int32_t penalty_last_n    = 64;
    float   penalty_repeat    = 0.0f;
    float   penalty_frequency = 0.0f;
    float   penalty_present   = 0.0f;

    // Filters & Selectors
    int32_t top_k          = 40;
    float   top_p          = 0.95f;
    size_t  top_p_min_keep = 1;
    float   min_p          = 0.05f;
    size_t  min_p_min_keep = 1;
    float   typical_p      = 1.0f;

    // Temperature
    float temperature = 0.8f;

    // Extended Temperature / XTC
    float temp_ext_delta    = 0.0f;
    float temp_ext_exponent = 0.0f;

    // Grammar & Constraints
    const vocab_t *vocab        = nullptr;
    const char    *grammar_str  = nullptr;
    const char    *grammar_root = nullptr;

    // Terminal Sampler (Greedy vs Stochastic)
    uint32_t seed = LLAMA_DEFAULT_SEED;

    bool force_greedy = false;

    [[nodiscard]] std::error_code validate() const noexcept
    {
        if(!std::isfinite(penalty_repeat) || !std::isfinite(penalty_frequency)
           || !std::isfinite(penalty_present) || !std::isfinite(top_p)
           || !std::isfinite(min_p) || !std::isfinite(typical_p)
           || !std::isfinite(temperature) || !std::isfinite(temp_ext_delta)
           || !std::isfinite(temp_ext_exponent))
        {
            return make_error_code(error_code::invalid_argument);
        }

        if(top_p <= 0.0f || top_p > 1.0f)
            return make_error_code(error_code::invalid_argument);

        if(min_p < 0.0f || min_p > 1.0f)
            return make_error_code(error_code::invalid_argument);

        if(typical_p < 0.0f || typical_p > 1.0f)
            return make_error_code(error_code::invalid_argument);

        if(temperature < 0.0f)
            return make_error_code(error_code::invalid_argument);

        if(temp_ext_delta < 0.0f)
            return make_error_code(error_code::invalid_argument);

        if(top_k < 0)
            return make_error_code(error_code::invalid_argument);

        if(penalty_last_n < 0)
            return make_error_code(error_code::invalid_argument);

        return make_error_code(error_code::success);
    }
};

/**
 * @brief Thread Safety Contract for `hj::llama::sampler`:
 *
 * - All Operations (sample(), accept(), reset(), reset_chain()): [NOT THREAD-SAFE]
 *   Samplers maintain internal mutable state (e.g., penalty token history, RNG state, 
 *   BNF grammar parsing state).
 * 
 * - Concurrency Pattern:
 *   Instantiate one `sampler` chain per sequence/request or store it in thread-local storage (TLS).
 *   Never share a `sampler` instance across dynamic concurrent requests without external locking.
 */
class sampler
{
  public:
    sampler() noexcept = default;
    explicit sampler(const sampler_options &opts,
                     sampler_chain_params_t chain_params =
                         llama_sampler_chain_default_params())
    {
        auto err = init(opts, chain_params);
        if(err)
        {
            throw std::runtime_error("hj::llama::sampler: " + err.message());
        }
    }

    ~sampler() = default;

    sampler(const sampler &)            = delete;
    sampler &operator=(const sampler &) = delete;

    sampler(sampler &&) noexcept            = default;
    sampler &operator=(sampler &&) noexcept = default;

    static sampler greedy()
    {
        sampler_options opts;
        opts.force_greedy = true;
        opts.temperature  = 0.0f;
        return sampler(opts);
    }

    std::error_code init(const sampler_options &opts,
                         sampler_chain_params_t chain_params =
                             llama_sampler_chain_default_params())
    {
        if(auto err = opts.validate(); err)
            return err;

        llama_sampler *raw_chain = llama_sampler_chain_init(chain_params);
        if(!raw_chain)
        {
            return make_error_code(error_code::sampler_init_failed);
        }

        std::unique_ptr<llama_sampler, detail::llama_sampler_deleter> chain(
            raw_chain);

        if(opts.vocab && opts.grammar_str && std::strlen(opts.grammar_str) > 0)
        {
            auto *g = llama_sampler_init_grammar(opts.vocab,
                                                 opts.grammar_str,
                                                 opts.grammar_root);
            if(!g)
                return make_error_code(error_code::sampler_init_failed);
            llama_sampler_chain_add(chain.get(), g);
        }

        if(opts.penalty_last_n > 0
           && (opts.penalty_repeat != 1.0f || opts.penalty_frequency != 0.0f
               || opts.penalty_present != 0.0f))
        {
            auto *p = llama_sampler_init_penalties(opts.penalty_last_n,
                                                   opts.penalty_repeat,
                                                   opts.penalty_frequency,
                                                   opts.penalty_present);
            if(!p)
                return make_error_code(error_code::sampler_init_failed);
            llama_sampler_chain_add(chain.get(), p);
        }

        if(opts.top_k > 0)
        {
            auto *tk = llama_sampler_init_top_k(opts.top_k);
            if(!tk)
                return make_error_code(error_code::sampler_init_failed);
            llama_sampler_chain_add(chain.get(), tk);
        }

        if(opts.typical_p < 1.0f)
        {
            auto *tp = llama_sampler_init_typical(opts.typical_p, 1);
            if(!tp)
                return make_error_code(error_code::sampler_init_failed);
            llama_sampler_chain_add(chain.get(), tp);
        }

        if(opts.top_p < 1.0f)
        {
            auto *tpp =
                llama_sampler_init_top_p(opts.top_p, opts.top_p_min_keep);
            if(!tpp)
                return make_error_code(error_code::sampler_init_failed);
            llama_sampler_chain_add(chain.get(), tpp);
        }

        if(opts.min_p > 0.0f)
        {
            auto *mp =
                llama_sampler_init_min_p(opts.min_p, opts.min_p_min_keep);
            if(!mp)
                return make_error_code(error_code::sampler_init_failed);
            llama_sampler_chain_add(chain.get(), mp);
        }

        if(opts.temperature > 0.0f && !opts.force_greedy)
        {
            llama_sampler *tm = nullptr;
            if(opts.temp_ext_delta > 0.0f)
            {
                tm = llama_sampler_init_temp_ext(opts.temperature,
                                                 opts.temp_ext_delta,
                                                 opts.temp_ext_exponent);
            } else
            {
                tm = llama_sampler_init_temp(opts.temperature);
            }
            if(!tm)
                return make_error_code(error_code::sampler_init_failed);
            llama_sampler_chain_add(chain.get(), tm);
        }

        llama_sampler *term = nullptr;
        if(opts.force_greedy || opts.temperature <= 0.0f)
        {
            term = llama_sampler_init_greedy();
        } else
        {
            term = llama_sampler_init_dist(opts.seed);
        }

        if(!term)
            return make_error_code(error_code::sampler_init_failed);

        llama_sampler_chain_add(chain.get(), term);
        _smpl = std::move(chain);
        return make_error_code(error_code::success);
    }

    llama_sampler *data() const noexcept { return _smpl.get(); }
    explicit       operator bool() const noexcept { return _smpl != nullptr; }

    void reset_chain() noexcept
    {
        if(_smpl)
            llama_sampler_reset(_smpl.get());
    }

    void reset() noexcept { reset_chain(); }

    token_t
    sample(context &ctx, const int32_t idx, std::error_code &err) noexcept
    {
        if(!_smpl)
        {
            err = make_error_code(error_code::invalid_state);
            return -1;
        }

        if(!ctx.data())
        {
            err = make_error_code(error_code::invalid_argument);
            return -1;
        }

        err.clear();
        return llama_sampler_sample(_smpl.get(), ctx.data(), idx);
    }

    std::error_code accept(const token_t token) noexcept
    {
        if(!_smpl)
            return make_error_code(error_code::invalid_state);

        llama_sampler_accept(_smpl.get(), token);
        return make_error_code(error_code::success);
    }

  private:
    std::unique_ptr<llama_sampler, detail::llama_sampler_deleter> _smpl{
        nullptr};
};

} // namespace llama
} // namespace hj

#endif // LLAMA_HPP