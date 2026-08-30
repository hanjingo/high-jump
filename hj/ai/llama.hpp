#ifndef LLAMA_HPP
#define LLAMA_HPP

#include <llama.h>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <stdexcept>

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

static inline void backend_init()
{
    llama_backend_init();
}

static inline void backend_free()
{
    llama_backend_free();
}

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

class backend
{
  public:
    backend() { detail::backend_mgr::instance().retain(); }

    ~backend() { detail::backend_mgr::instance().release(); }

    backend(const backend &)            = delete;
    backend &operator=(const backend &) = delete;

    backend(backend &&other) noexcept
        : _owner(other._owner)
    {
        other._owner = false;
    }

    backend &operator=(backend &&other) noexcept
    {
        if(this != &other)
        {
            if(_owner)
            {
                detail::backend_mgr::instance().release();
            }
            _owner       = other._owner;
            other._owner = false;
        }
        return *this;
    }

  private:
    bool _owner{true};
};

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

        _capacity = n_tokens;
        _batch    = llama_batch_init(n_tokens, embd, n_seq_max);
    }

    explicit batch(const std::vector<token_t> &tokens)
    {
        if(tokens.empty())
        {
            throw std::invalid_argument(
                "hj::llama::batch: Token vector cannot be empty");
        }

        _capacity = static_cast<int32_t>(tokens.size());
        _batch    = llama_batch_init(_capacity, 0, 1);
        auto err  = set_tokens(tokens.data(), _capacity, 0);
        if(err)
        {
            free_batch();
            throw std::invalid_argument(err.message());
        }
    }

    batch(const token_t *tokens, int32_t n_tokens)
    {
        if(!tokens || n_tokens <= 0)
        {
            throw std::invalid_argument("hj::llama::batch: Invalid token "
                                        "pointer or non-positive n_tokens");
        }

        _capacity = n_tokens;
        _batch    = llama_batch_init(n_tokens, 0, 1);
        auto err  = set_tokens(tokens, n_tokens, 0);
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

    llama_batch       &get() { return _batch; }
    const llama_batch &get() const { return _batch; }

    int32_t            size() const noexcept { return _batch.n_tokens; }
    int32_t            capacity() const noexcept { return _capacity; }
    bool               empty() const noexcept { return _batch.n_tokens == 0; }
    llama_batch       *data() noexcept { return &_batch; }
    const llama_batch *data() const noexcept { return &_batch; }

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

class model
{
  public:
    model() = default;

    explicit model(llama_model *m)
        : _model(m)
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
        : _model(llama_model_init_from_user(
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

    // std::error_code load(const std::filesystem::path &filename,
    //                      model_params_t params = llama_model_default_params())
    // {
    //     auto err = detail::validate_file_path(filename);
    //     if(err)
    //         return err;

    //     _backend_guard = std::make_shared<backend>();
    //     auto loaded =
    //         llama_model_load_from_file(filename.string().c_str(), params);
    //     if(!loaded)
    //         return make_error_code(error_code::invalid_model);

    //     _model.reset(loaded);
    //     return err;
    // }

    std::error_code load(const std::filesystem::path &filename,
                         model_params_t params = llama_model_default_params())
    {
        auto err = detail::validate_file_path(filename);
        if(err)
            return err;

        auto loaded =
            llama_model_load_from_file(filename.string().c_str(), params);

        if(!loaded)
            return make_error_code(error_code::invalid_model);

        _model.reset(loaded);
        return {};
    }

    std::error_code load(FILE          *file,
                         model_params_t params = llama_model_default_params())
    {
        if(file == nullptr)
            return make_error_code(error_code::invalid_path);

        _backend_guard = std::make_shared<backend>();
        auto loaded    = llama_model_load_from_file_ptr(file, params);
        if(!loaded)
            return make_error_code(error_code::invalid_model);

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

        _backend_guard = std::make_shared<backend>();
        auto loaded    = llama_model_load_from_splits(paths, n_paths, params);
        if(!loaded)
            return make_error_code(error_code::invalid_model);

        _model.reset(loaded);
        return {};
    }

    std::error_code save(const std::filesystem::path &filename) const
    {
        if(!_model)
            return make_error_code(error_code::invalid_model);

        if(filename.empty())
            return make_error_code(error_code::invalid_path);

        llama_model_save_to_file(_model.get(), filename.string().c_str());

        auto err = detail::validate_file_path(filename);
        if(err)
            return err;

        return {};
    }

    [[deprecated("Manual freeing compromises associated contexts. Rely on RAII "
                 "scope instead.")]]
    bool free()
    {
        if(!_model)
            return false;

        _model.reset();
        return true;
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

        auto vocab = llama_model_get_vocab(_model.get());
        if(!vocab)
        {
            err = make_error_code(error_code::invalid_state);
            return {};
        }

        if(prompt.empty())
        {
            err = make_error_code(error_code::success);
            return {};
        }

        auto n_tokens = llama_tokenize(vocab,
                                       prompt.c_str(),
                                       static_cast<int32_t>(prompt.length()),
                                       nullptr,
                                       0,
                                       add_special,
                                       parse_special);
        n_tokens      = std::abs(n_tokens);
        if(n_tokens <= 0)
        {
            err = make_error_code(error_code::backend_error);
            return {};
        }

        std::vector<token_t> tokens(n_tokens);
        n_tokens = llama_tokenize(vocab,
                                  prompt.c_str(),
                                  static_cast<int32_t>(prompt.length()),
                                  tokens.data(),
                                  static_cast<int32_t>(tokens.size()),
                                  add_special,
                                  parse_special);
        n_tokens = std::abs(n_tokens);
        tokens.resize(n_tokens);
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
    std::shared_ptr<backend> _backend_guard{nullptr};
    std::unique_ptr<llama_model, detail::llama_model_deleter> _model{nullptr};
};

class adapter_lora
{
  public:
    adapter_lora() = default;

    adapter_lora(const model &m, const char *path_lora)
        : _adapter((m.data() == nullptr || path_lora == nullptr)
                       ? nullptr
                       : llama_adapter_lora_init(m.data(), path_lora))
    {
    }

    explicit adapter_lora(adapter_lora_t *adapter)
        : _adapter(adapter)
    {
    }

    ~adapter_lora() = default;

    adapter_lora(const adapter_lora &)            = delete;
    adapter_lora &operator=(const adapter_lora &) = delete;

    adapter_lora(adapter_lora &&) noexcept            = default;
    adapter_lora &operator=(adapter_lora &&) noexcept = default;

    adapter_lora_t *data() const { return _adapter.get(); }

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
    std::unique_ptr<adapter_lora_t, detail::llama_adapter_lora_deleter>
        _adapter{nullptr};
};

/**
 * @brief Wrapper around llama_context.
 * 
 * @note **Lifetime Contract**:
 * `context` does NOT own the underlying `model`. The `model` instance MUST 
 * remain valid and outlive the `context` for its entire lifetime.
 * Accessing `context` after the associated `model` is destructed or freed 
 * results in undefined behavior.
 */
class context
{
  public:
    context() = default;

    context(const model &m, context_params_t params) { init(m, params); }

    context(const model *m, context_params_t params) { init(m, params); }

    ~context() = default;

    context(const context &)            = delete;
    context &operator=(const context &) = delete;

    context(context &&) noexcept            = default;
    context &operator=(context &&) noexcept = default;

    static context_params_t default_params()
    {
        return llama_context_default_params();
    }

    llama_context *data() const { return _ctx.get(); }

    explicit operator bool() const noexcept { return _ctx != nullptr; }

    const llama_model *get_model() const
    {
        return _ctx ? llama_get_model(_ctx.get()) : nullptr;
    }

    /// @brief Re-initializes the context with a new model/params.
    /// @details Guarantees Strong Exception Safety: If initialization fails,
    ///          the old context state is completely untouched.
    /// @return std::error_code Indicating success or the failure reason.
    std::error_code init(const model &m, context_params_t params)
    {
        if(!m.data())
        {
            return make_error_code(error_code::invalid_argument);
        }

        // Attempt new allocation before invalidating current resource
        llama_context *new_raw = llama_init_from_model(m.data(), params);
        if(!new_raw)
        {
            return make_error_code(error_code::backend_error);
        }

        _ctx.reset(new_raw);
        return make_error_code(error_code::success);
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
};

class sampler
{
  public:
    sampler() = default;

    explicit sampler(const sampler_options &opts,
                     sampler_chain_params_t chain_params =
                         llama_sampler_chain_default_params())
    {
        _smpl.reset(llama_sampler_chain_init(chain_params));
        if(!_smpl)
            return;

        if(opts.vocab && opts.grammar_str && strlen(opts.grammar_str) > 0)
        {
            llama_sampler_chain_add(
                _smpl.get(),
                llama_sampler_init_grammar(opts.vocab,
                                           opts.grammar_str,
                                           opts.grammar_root));
        }

        if(opts.penalty_last_n > 0
           && (opts.penalty_repeat != 1.0f || opts.penalty_frequency != 0.0f
               || opts.penalty_present != 0.0f))
        {
            llama_sampler_chain_add(
                _smpl.get(),
                llama_sampler_init_penalties(opts.penalty_last_n,
                                             opts.penalty_repeat,
                                             opts.penalty_frequency,
                                             opts.penalty_present));
        }

        if(opts.top_k > 0)
        {
            llama_sampler_chain_add(_smpl.get(),
                                    llama_sampler_init_top_k(opts.top_k));
        }
        if(opts.typical_p < 1.0f)
        {
            llama_sampler_chain_add(
                _smpl.get(),
                llama_sampler_init_typical(opts.typical_p, 1));
        }
        if(opts.top_p < 1.0f)
        {
            llama_sampler_chain_add(
                _smpl.get(),
                llama_sampler_init_top_p(opts.top_p, opts.top_p_min_keep));
        }
        if(opts.min_p > 0.0f)
        {
            llama_sampler_chain_add(
                _smpl.get(),
                llama_sampler_init_min_p(opts.min_p, opts.min_p_min_keep));
        }

        if(opts.temperature > 0.0f && !opts.force_greedy)
        {
            if(opts.temp_ext_delta > 0.0f)
            {
                llama_sampler_chain_add(
                    _smpl.get(),
                    llama_sampler_init_temp_ext(opts.temperature,
                                                opts.temp_ext_delta,
                                                opts.temp_ext_exponent));
            } else
            {
                llama_sampler_chain_add(
                    _smpl.get(),
                    llama_sampler_init_temp(opts.temperature));
            }
        }

        if(opts.force_greedy || opts.temperature <= 0.0f)
        {
            llama_sampler_chain_add(_smpl.get(), llama_sampler_init_greedy());
        } else
        {
            llama_sampler_chain_add(_smpl.get(),
                                    llama_sampler_init_dist(opts.seed));
        }
    }

    static sampler greedy()
    {
        sampler_options opts;
        opts.force_greedy = true;
        opts.temperature  = 0.0f;
        return sampler(opts);
    }

    ~sampler() = default;

    sampler(const sampler &)            = delete;
    sampler &operator=(const sampler &) = delete;

    sampler(sampler &&) noexcept            = default;
    sampler &operator=(sampler &&) noexcept = default;

    llama_sampler *data() const { return _smpl.get(); }

    void reset()
    {
        if(_smpl)
            llama_sampler_reset(_smpl.get());
    }

    token_t sample(context &ctx, const int32_t idx, std::error_code &err)
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

        return llama_sampler_sample(_smpl.get(), ctx.data(), idx);
    }

    std::error_code accept(const token_t token)
    {
        if(!_smpl)
            return make_error_code(error_code::invalid_state);

        llama_sampler_accept(_smpl.get(), token);
        return {};
    }

  private:
    std::unique_ptr<llama_sampler, detail::llama_sampler_deleter> _smpl{
        nullptr};
};

} // namespace llama
} // namespace hj

#endif // LLAMA_HPP