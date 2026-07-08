#ifndef LLAMA_HPP
#define LLAMA_HPP

#include <llama.h>
#include <memory>
#include <cstdlib>

#include <boost/filesystem.hpp>

#ifndef LLAMA_MODEL_DESC_SIZE
#define LLAMA_MODEL_DESC_SIZE 2048
#endif

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

static inline void backend_init()
{
    return llama_backend_init();
}
static inline void backend_free()
{
    return llama_backend_free();
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

class batch
{
  public:
    batch(int32_t n_tokens, int32_t embd, int32_t n_seq_max)
        : _capacity(n_tokens)
        , _batch{llama_batch_init(n_tokens, embd, n_seq_max)}
    {
    }

    batch(const std::vector<token_t> &tokens)
        : _capacity(static_cast<int32_t>(tokens.size()))
        , _batch{llama_batch_init(_capacity, 0, 1)}
    {
        set_tokens(tokens.data(), _capacity, 0);
    }

    batch(const token_t *tokens, int32_t n_tokens)
        : _capacity(n_tokens)
        , _batch{llama_batch_init(n_tokens, 0, 1)}
    {
        set_tokens(tokens, n_tokens, 0);
    }

    ~batch()
    {
        if(_batch.token != nullptr || _batch.embd != nullptr)
            llama_batch_free(_batch);
    }

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
            if(_batch.token != nullptr || _batch.embd != nullptr)
                llama_batch_free(_batch);

            _batch    = other._batch;
            _capacity = other._capacity;

            std::memset(&other._batch, 0, sizeof(llama_batch));
            other._capacity = 0;
        }
        return *this;
    }

    llama_batch       &get() { return _batch; }
    const llama_batch &get() const { return _batch; }

    operator llama_batch &() { return _batch; }
    operator const llama_batch &() const { return _batch; }

    void set_logits(int32_t idx, bool value)
    {
        if(idx >= 0 && idx < _batch.n_tokens)
            _batch.logits[idx] = value;
    }

    void
    set_tokens(const token_t *tokens, int32_t n_tokens, int32_t start_pos = 0)
    {
        int32_t safe_tokens = std::min(n_tokens, _capacity);
        for(int32_t i = 0; i < safe_tokens; ++i)
        {
            _batch.token[i]     = tokens[i];
            _batch.pos[i]       = start_pos + i;
            _batch.n_seq_id[i]  = 1;
            _batch.seq_id[i][0] = 0;
            _batch.logits[i]    = false;
        }
        _batch.n_tokens = safe_tokens;
    }

  private:
    int32_t     _capacity = 0;
    llama_batch _batch{};
};

class memory
{
  public:
    memory()
        : _mem(nullptr)
    {
    }
    memory(llama_memory_t mem)
        : _mem(mem)
    {
    }
    ~memory() { clear(true); }

    void clear(bool data)
    {
        if(_mem)
            llama_memory_clear(_mem, data);
    }

    void seq_rm(seq_id_t seq_id, pos_t p0, pos_t p1)
    {
        if(_mem)
            llama_memory_seq_rm(_mem, seq_id, p0, p1);
    }

    void seq_cp(seq_id_t seq_id_src, seq_id_t seq_id_dst, pos_t p0, pos_t p1)
    {
        if(_mem)
            llama_memory_seq_cp(_mem, seq_id_src, seq_id_dst, p0, p1);
    }

    void seq_keep(seq_id_t seq_id)
    {
        if(_mem)
            llama_memory_seq_keep(_mem, seq_id);
    }

    void seq_add(seq_id_t seq_id, pos_t p0, pos_t p1, pos_t delta)
    {
        if(_mem)
            llama_memory_seq_add(_mem, seq_id, p0, p1, delta);
    }

    void seq_div(seq_id_t seq_id, pos_t p0, pos_t p1, int d)
    {
        if(_mem)
            llama_memory_seq_div(_mem, seq_id, p0, p1, d);
    }

    void seq_pos_min(seq_id_t seq_id)
    {
        if(_mem)
            llama_memory_seq_pos_min(_mem, seq_id);
    }

    void seq_pos_max(seq_id_t seq_id)
    {
        if(_mem)
            llama_memory_seq_pos_max(_mem, seq_id);
    }

    bool can_shift() const
    {
        return _mem ? llama_memory_can_shift(_mem) : false;
    }

  private:
    llama_memory_t _mem;
};

class model
{
  public:
    model()
        : _model(nullptr)
    {
    }
    model(llama_model *m)
        : _model(m)
    {
    }
    model(const char    *filename,
          model_params_t params = llama_model_default_params())
        : _model(llama_model_load_from_file(filename, params))
    {
    }
    model(FILE *file, model_params_t params = llama_model_default_params())
        : _model(llama_model_load_from_file_ptr(file, params))
    {
    }
    model(const char   **paths,
          size_t         n_paths,
          model_params_t params = llama_model_default_params())
        : _model(llama_model_load_from_splits(paths, n_paths, params))
    {
    }
    model(gguf_ctx_t        *metadata,
          set_tensor_data_fn set_tensor_data,
          void              *set_tensor_data_ud,
          model_params_t     params)
        : _model(llama_model_init_from_user(
              metadata, set_tensor_data, set_tensor_data_ud, params))
    {
    }
    model(const model &)            = delete;
    model &operator=(const model &) = delete;
    model(model &&other) noexcept
        : _model(other._model)
    {
        other._model = nullptr;
    }
    ~model()
    {
        if(_model)
            llama_model_free(_model);
    }

    static model_params_t default_params()
    {
        return llama_model_default_params();
    }

    inline llama_model *data() { return _model; }

    inline const llama_model *const_data() const { return _model; }

    bool load(const char    *filename,
              model_params_t params = llama_model_default_params())
    {
        if(!boost::filesystem::exists(boost::filesystem::path(filename)))
            return false;

        if(_model)
            llama_model_free(_model);

        auto model = llama_model_load_from_file(filename, params);
        _model     = (model != nullptr) ? model : _model;
        return _model != nullptr;
    }
    bool load(FILE *file, model_params_t params = llama_model_default_params())
    {
        if(file == nullptr)
            return false;

        if(_model)
            llama_model_free(_model);

        auto model = llama_model_load_from_file_ptr(file, params);
        _model     = (model != nullptr) ? model : _model;
        return _model != nullptr;
    }
    bool load(const char   **paths,
              size_t         n_paths,
              model_params_t params = llama_model_default_params())
    {
        if(_model)
            llama_model_free(_model);

        auto model = llama_model_load_from_splits(paths, n_paths, params);
        _model     = (model != nullptr) ? model : _model;
        return _model != nullptr;
    }
    bool save(const char *filename)
    {
        if(!_model)
            return false;

        llama_model_save_to_file(_model, filename);
        return true;
    }
    bool free()
    {
        if(!_model)
            return false;

        llama_model_free(_model);
        _model = nullptr;
        return true;
    }
    std::vector<token_t>
    tokenize(const std::string &prompt, bool add_special, bool parse_special)
    {
        if(!_model)
            return {};

        auto vocab = llama_model_get_vocab(_model);
        if(!vocab)
            return {};

        auto n_tokens = llama_tokenize(vocab,
                                       prompt.c_str(),
                                       prompt.length(),
                                       nullptr,
                                       0,
                                       add_special,
                                       parse_special);
        n_tokens      = std::abs(n_tokens);
        if(n_tokens <= 0)
            return {};

        std::vector<token_t> tokens(n_tokens);
        n_tokens = llama_tokenize(vocab,
                                  prompt.c_str(),
                                  prompt.length(),
                                  tokens.data(),
                                  tokens.size(),
                                  add_special,
                                  parse_special);
        n_tokens = std::abs(n_tokens);
        tokens.resize(n_tokens);
        return tokens;
    }
    uint64_t    size() { return (_model) ? llama_model_size(_model) : 0; }
    std::string desc()
    {
        if(!_model)
            return std::string();

        char buf[LLAMA_MODEL_DESC_SIZE] = {0};
        auto sz = llama_model_desc(_model, buf, sizeof(buf));
        return std::string(buf, sz);
    }

    int32_t n_ctx_train()
    {
        if(!_model)
            return 0;

        return llama_model_n_ctx_train(_model);
    }
    int32_t n_embd()
    {
        if(!_model)
            return 0;

        return llama_model_n_embd(_model);
    }
    int32_t n_embd_inp()
    {
        if(!_model)
            return 0;

        return llama_model_n_embd_inp(_model);
    }
    int32_t n_embd_out()
    {
        if(!_model)
            return 0;

        return llama_model_n_embd_out(_model);
    }
    int32_t n_layer()
    {
        if(!_model)
            return 0;

        return llama_model_n_layer(_model);
    }
    int32_t n_head()
    {
        if(!_model)
            return 0;

        return llama_model_n_head(_model);
    }
    int32_t n_head_kv()
    {
        if(!_model)
            return 0;

        return llama_model_n_head_kv(_model);
    }
    int32_t n_swa()
    {
        if(!_model)
            return 0;

        return llama_model_n_swa(_model);
    }

    const vocab_t *get_vocab()
    {
        if(!_model)
            return nullptr;

        return llama_model_get_vocab(const_cast<llama_model *>(_model));
    }
    int n_vocab()
    {
        if(!_model)
            return 0;

        return llama_n_vocab(
            llama_model_get_vocab(const_cast<llama_model *>(_model)));
    }
    bool token_is_eog(token_t token)
    {
        if(!_model)
            return false;

        return llama_token_is_eog(
            llama_model_get_vocab(const_cast<llama_model *>(_model)),
            token);
    }
    int32_t token_to_piece(
        token_t token, char *buf, int32_t length, int32_t lstrip, bool special)
    {
        if(!_model)
            return -1;

        return llama_token_to_piece(
            llama_model_get_vocab(const_cast<llama_model *>(_model)),
            token,
            buf,
            length,
            lstrip,
            special);
    }
    llama_rope_type rope_type()
    {
        if(!_model)
            return LLAMA_ROPE_TYPE_NONE;

        return llama_model_rope_type(const_cast<llama_model *>(_model));
    }

    float rope_freq_scale_train()
    {
        if(!_model)
            return 0.0f;

        return llama_model_rope_freq_scale_train(_model);
    }

    uint32_t n_cls_out()
    {
        if(!_model)
            return 0;

        return llama_model_n_cls_out(const_cast<llama_model *>(_model));
    }

    const char *cls_label(uint32_t i)
    {
        if(!_model)
            return nullptr;

        return llama_model_cls_label(const_cast<llama_model *>(_model), i);
    }

    int32_t meta_val_str(const char *key, char *buf, size_t buf_size)
    {
        if(!_model)
            return -1;

        return llama_model_meta_val_str(const_cast<llama_model *>(_model),
                                        key,
                                        buf,
                                        buf_size);
    }

    int32_t meta_count()
    {
        if(!_model)
            return -1;

        return llama_model_meta_count(const_cast<llama_model *>(_model));
    }

    int32_t meta_key_by_index(int32_t i, char *buf, size_t buf_size)
    {
        if(!_model)
            return -1;

        return llama_model_meta_key_by_index(const_cast<llama_model *>(_model),
                                             i,
                                             buf,
                                             buf_size);
    }

    const std::string chat_template(const char *name)
    {
        return (_model) ? std::string(llama_model_chat_template(_model, name))
                        : std::string();
    }

    uint64_t n_params()
    {
        if(!_model)
            return 0;

        return llama_model_n_params(const_cast<llama_model *>(_model));
    }

    bool has_encoder()
    {
        if(!_model)
            return false;

        return llama_model_has_encoder(const_cast<llama_model *>(_model));
    }
    bool has_decoder()
    {
        if(!_model)
            return false;

        return llama_model_has_decoder(const_cast<llama_model *>(_model));
    }

    token_t decoder_start_token()
    {
        if(!_model)
            return -1;

        return llama_model_decoder_start_token(
            const_cast<llama_model *>(_model));
    }

    bool is_recurrent()
    {
        if(!_model)
            return false;

        return llama_model_is_recurrent(const_cast<llama_model *>(_model));
    }

    bool is_hybrid()
    {
        if(!_model)
            return false;

        return llama_model_is_hybrid(const_cast<llama_model *>(_model));
    }

    bool is_diffusion()
    {
        if(!_model)
            return false;

        return llama_model_is_diffusion(const_cast<llama_model *>(_model));
    }

  private:
    llama_model *_model;
};

class adapter_lora
{
  public:
    adapter_lora()
        : _adapter(nullptr)
    {
    }
    adapter_lora(model &m, const char *path_lora)
        : _adapter((m.data() == nullptr || path_lora == nullptr)
                       ? nullptr
                       : llama_adapter_lora_init(m.data(), path_lora))
    {
    }
    adapter_lora(adapter_lora_t *adapter)
        : _adapter(adapter)
    {
    }
    ~adapter_lora()
    {
        if(_adapter)
            llama_adapter_lora_free(_adapter);
    }

    inline adapter_lora_t *data() { return _adapter; }

    int32_t meta_val_str(const char *key, char *buf, size_t buf_size)
    {
        if(!_adapter)
            return -1;

        return llama_adapter_meta_val_str(_adapter, key, buf, buf_size);
    }

    int32_t meta_cout()
    {
        if(!_adapter)
            return -1;

        return llama_adapter_meta_count(_adapter);
    }

    int32_t meta_key_by_index(int32_t i, char *buf, size_t buf_size)
    {
        if(!_adapter)
            return -1;

        return llama_adapter_meta_key_by_index(_adapter, i, buf, buf_size);
    }

    uint64_t meta_val_str_by_index(int32_t i, char *buf, size_t buf_size)
    {
        if(!_adapter)
            return 0;

        return llama_adapter_meta_val_str_by_index(_adapter, i, buf, buf_size);
    }

    uint64_t get_alora_n_invocation_tokens()
    {
        if(!_adapter)
            return 0;

        return llama_adapter_get_alora_n_invocation_tokens(_adapter);
    }

    const token_t *get_alora_invocation_tokens()
    {
        if(!_adapter)
            return nullptr;

        return llama_adapter_get_alora_invocation_tokens(_adapter);
    }

  private:
    adapter_lora_t *_adapter;
};

class context
{
  public:
    context()
        : _ctx{nullptr}
    {
    }
    context(model &m, context_params_t params)
        : _ctx{llama_init_from_model(m.data(), params)}
    {
    }
    context(model *m, context_params_t params)
        : _ctx{(m == nullptr) ? nullptr
                              : llama_init_from_model(m->data(), params)}
    {
    }
    ~context()
    {
        if(_ctx)
            llama_free(_ctx);
    }

    static context_params_t default_params()
    {
        return llama_context_default_params();
    }

    llama_context *const data() { return _ctx; }

    void init(model &m, context_params_t params)
    {
        if(_ctx)
            llama_free(_ctx);

        _ctx = llama_init_from_model(m.data(), params);
    }

    std::shared_ptr<model> get_model()
    {
        if(!_ctx)
            return nullptr;

        return std::make_shared<model>(
            const_cast<llama_model *>(llama_get_model(_ctx)));
    }

    std::shared_ptr<memory> get_memory()
    {
        if(!_ctx)
            return nullptr;

        return std::make_shared<memory>(llama_get_memory(_ctx));
    }


    bool attach_threadpool(threadpool_t threadpool,
                           threadpool_t threadpool_batch)
    {
        if(!_ctx)
            return false;

        llama_attach_threadpool(_ctx, threadpool, threadpool_batch);
        return true;
    }

    bool detach_threadpool()
    {
        if(!_ctx)
            return false;

        llama_detach_threadpool(_ctx);
        return true;
    }

    uint32_t n_ctx() const { return _ctx ? llama_n_ctx(_ctx) : 0; }
    uint32_t n_ctx_seq() const { return _ctx ? llama_n_ctx_seq(_ctx) : 0; }
    uint32_t n_batch() const { return _ctx ? llama_n_batch(_ctx) : 0; }
    uint32_t n_ubatch() const { return _ctx ? llama_n_ubatch(_ctx) : 0; }
    uint32_t n_seq_max() const { return _ctx ? llama_n_seq_max(_ctx) : 0; }
    // uint32_t n_rs_seq() const { return _ctx ? llama_n_rs_seq(_ctx) : 0; }

    int32_t set_adapter_lora(std::vector<adapter_lora> &loras, float *scales)
    {
        if(!_ctx)
            return -1;

        std::vector<adapter_lora_t *> lora_ptrs;
        for(auto &lora : loras)
        {
            if(!lora.data())
                return -1;

            lora_ptrs.push_back(lora.data());
        }

        return llama_set_adapters_lora(_ctx,
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

        return llama_set_adapter_cvec(_ctx,
                                      data,
                                      len,
                                      n_embd,
                                      il_start,
                                      il_end);
    }

    int32_t decode(llama_batch batch)
    {
        if(!_ctx)
            return -1;

        return llama_decode(_ctx, batch);
    }

    int32_t decode(const batch &batch)
    {
        if(!_ctx)
            return -1;

        return llama_decode(_ctx, batch.get());
    }

    const float *get_logits_ith(int32_t i)
    {
        if(!_ctx)
            return nullptr;

        return llama_get_logits_ith(_ctx, i);
    }

  private:
    llama_context *_ctx;
};

class sampler
{
  public:
    struct params
    {
        int32_t penalty_last_n    = 64;
        float   penalty_repeat    = 0.0;
        float   penalty_frequency = 0.0;
        float   penalty_present   = 0.0;

        float temperature              = 0.0;
        float temperature_ext          = 0.0;
        float temperature_ext_delta    = 0.0;
        float temperature_ext_exponent = 0.0;

        uint32_t seed = 0;

        int32_t top_k          = 0;
        float   top_p          = 0.0;
        int32_t top_p_min_keep = 0;
        float   min_p          = 0.0;
        int32_t min_p_min_keep = 0;

        // add more param laters as needed
    };

  public:
    sampler()
        : _smpl{llama_sampler_chain_init(llama_sampler_chain_default_params())}
    {
    }

    sampler(sampler_chain_params_t chain_params)
        : _smpl{llama_sampler_chain_init(chain_params)}
    {
    }

    sampler(sampler_chain_params_t chain_params, params sampler_params)
        : _smpl{llama_sampler_chain_init(chain_params)}
    {
        if(!_smpl)
            return;

        // init penalty first (MUST)
        if(sampler_params.penalty_last_n > 0
           && (sampler_params.penalty_repeat != 1.0f
               || sampler_params.penalty_frequency != 0.0f
               || sampler_params.penalty_present != 0.0f))
            init_penalties(sampler_params.penalty_last_n,
                           sampler_params.penalty_repeat,
                           sampler_params.penalty_frequency,
                           sampler_params.penalty_present);

        // init top-k, top-p, min-p before temperature init (MUST)
        if(sampler_params.top_k > 0)
            init_top_k(sampler_params.top_k);

        if(sampler_params.top_p > 0.0f && sampler_params.top_p < 1.0f)
            init_top_p(sampler_params.top_p,
                       sampler_params.top_p_min_keep > 0
                           ? sampler_params.top_p_min_keep
                           : 1);

        if(sampler_params.min_p > 0.0f && sampler_params.min_p < 1.0f)
            init_min_p(sampler_params.min_p,
                       sampler_params.min_p_min_keep > 0
                           ? sampler_params.min_p_min_keep
                           : 1);

        // init temperature after top-k/top-p (MUST)
        if(sampler_params.temperature > 0.0f)
            init_temp(sampler_params.temperature);

        if(sampler_params.temperature_ext > 0.0f)
            init_temp_ext(sampler_params.temperature_ext,
                          sampler_params.temperature_ext_delta,
                          sampler_params.temperature_ext_exponent);

        // init seed last (MUST)
        init_dist(sampler_params.seed);
    }

    ~sampler()
    {
        if(_smpl)
            llama_sampler_free(_smpl);
    }

    static sampler_chain_params_t default_chain_params()
    {
        return llama_sampler_chain_default_params();
    }

    llama_sampler *data() { return _smpl; }

    void init_greedy()
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_greedy());
    }

    void init_dist(uint32_t seed)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_dist(seed));
    }

    void init_top_k(int32_t top_k)
    {
        if(!_smpl || top_k <= 0)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_top_k(top_k));
    }

    void init_top_p(float p, size_t min_keep)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_top_p(p, min_keep));
    }

    void init_min_p(float p, size_t min_keep)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_min_p(p, min_keep));
    }

    void init_typical(float p, size_t min_keep)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_typical(p, min_keep));
    }

    void init_temp(float temp)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_temp(temp));
    }

    void init_temp_ext(float t, float delta, float exponent)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(
            _smpl,
            llama_sampler_init_temp_ext(t, delta, exponent));
    }

    void init_xtc(float p, float t, size_t min_keep, uint32_t seed)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl,
                                llama_sampler_init_xtc(p, t, min_keep, seed));
    }

    void init_top_n_sigma(float n)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_top_n_sigma(n));
    }

    void init_mirostat(
        int32_t n_vocab, uint32_t seed, float tau, float eta, int32_t m)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(
            _smpl,
            llama_sampler_init_mirostat(n_vocab, seed, tau, eta, m));
    }

    void init_mirostat_v2(uint32_t seed, float tau, float eta)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl,
                                llama_sampler_init_mirostat_v2(seed, tau, eta));
    }

    void init_grammar(const vocab_t *vocab,
                      const char    *grammar_str,
                      const char    *grammar_root)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(
            _smpl,
            llama_sampler_init_grammar(vocab, grammar_str, grammar_root));
    }

    void init_grammar_lazy(const vocab_t *vocab,
                           const char    *grammar_str,
                           const char    *grammar_root,
                           const char   **trigger_words,
                           size_t         num_trigger_words,
                           const token_t *trigger_tokens,
                           size_t         num_trigger_tokens)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(
            _smpl,
            llama_sampler_init_grammar_lazy(vocab,
                                            grammar_str,
                                            grammar_root,
                                            trigger_words,
                                            num_trigger_words,
                                            trigger_tokens,
                                            num_trigger_tokens));
    }

    void init_grammar_lay_patterns(const vocab_t *vocab,
                                   const char    *grammar_str,
                                   const char    *grammar_root,
                                   const char   **trigger_patterns,
                                   size_t         num_trigger_patterns,
                                   const token_t *trigger_tokens,
                                   size_t         num_trigger_tokens)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(
            _smpl,
            llama_sampler_init_grammar_lazy_patterns(vocab,
                                                     grammar_str,
                                                     grammar_root,
                                                     trigger_patterns,
                                                     num_trigger_patterns,
                                                     trigger_tokens,
                                                     num_trigger_tokens));
    }

    void init_penalties(int32_t penalty_last_n,
                        float   penalty_repeat,
                        float   penalty_freq,
                        float   penalty_present)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl,
                                llama_sampler_init_penalties(penalty_last_n,
                                                             penalty_repeat,
                                                             penalty_freq,
                                                             penalty_present));
    }

    void init_sampler_init_dry(const vocab_t *vocab,
                               int32_t        n_ctx_train,
                               float          dry_multiplier,
                               float          dry_base,
                               int32_t        dry_allowed_length,
                               int32_t        dry_penalty_last_n,
                               const char   **seq_breakers,
                               size_t         num_breakers)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl,
                                llama_sampler_init_dry(vocab,
                                                       n_ctx_train,
                                                       dry_multiplier,
                                                       dry_base,
                                                       dry_allowed_length,
                                                       dry_penalty_last_n,
                                                       seq_breakers,
                                                       num_breakers));
    }

    void init_adaptive_p(float target, float decay, uint32_t seed)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(
            _smpl,
            llama_sampler_init_adaptive_p(target, decay, seed));
    }

    void init_logit_bias(int32_t                 n_vocab,
                         int32_t                 n_logit_bias,
                         const llama_logit_bias *logit_bias)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(
            _smpl,
            llama_sampler_init_logit_bias(n_vocab, n_logit_bias, logit_bias));
    }

    void init_infill(const vocab_t *vocab)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_infill(vocab));
    }

    void reset()
    {
        if(!_smpl)
            return;

        llama_sampler_reset(_smpl);
    }

    token_t sample(context &ctx, const int32_t idx)
    {
        if(!_smpl || !ctx.data())
            return -1;

        return llama_sampler_sample(_smpl, ctx.data(), idx);
    }

    void accept(const token_t token)
    {
        if(!_smpl)
            return;

        llama_sampler_accept(_smpl, token);
    }

  private:
    llama_sampler *_smpl;
};

} // namespace llama
} // namespace hj

#endif // LLAMA_HPP