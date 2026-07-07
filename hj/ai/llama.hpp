#ifndef LLAMA_HPP
#define LLAMA_HPP

#include <llama.h>
#include <memory>

#include <boost/filesystem.hpp>

#ifndef LLAMA_MODEL_DESC_SIZE
#define LLAMA_MODEL_DESC_SIZE 2048
#endif

namespace hj
{

namespace llama
{
using model_params_t   = llama_model_params;
using context_params_t = llama_context_params;
using sampler_params_t = llama_sampler_chain_params;

using vocab_t        = llama_vocab;
using memory_t       = llama_memory_t;
using threadpool_t   = ggml_threadpool_t;
using gguf_ctx_t     = gguf_context;
using adapter_lora_t = llama_adapter_lora;
using batch_t        = llama_batch;

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
    batch()
        : _batch{llama_batch_init(0, 0, 0)}
    {
    }
    batch(batch_t &&bt)
        : _batch{std::move(bt)}
    {
    }
    batch(int32_t n_tokens, int32_t embd, int32_t n_seq_max)
        : _batch(llama_batch_init(n_tokens, embd, n_seq_max))
    {
    }
    batch(const batch &)            = delete;
    batch &operator=(const batch &) = delete;
    batch &operator=(batch &&other) noexcept
    {
        if(this != &other)
        {
            llama_batch_free(_batch);
            _batch       = other._batch;
            other._batch = llama_batch_init(0, 0, 0);
        }
        return *this;
    }
    batch(batch &&other) noexcept
        : _batch(other._batch)
    {
    }

    ~batch() { llama_batch_free(_batch); }
    batch_t *operator->() { return &_batch; }
    batch_t &data() { return _batch; }

    static batch get_one(token_t *tokens, int32_t n_tokens)
    {
        auto bt = llama_batch_get_one(tokens, n_tokens);
        return batch(std::move(bt));
    }

    static batch get_one(std::vector<token_t> &tokens)
    {
        auto bt = llama_batch_get_one(tokens.data(), tokens.size());
        return batch(std::move(bt));
    }

    inline void set_n_tokens(size_t n_tokens) { _batch.n_tokens = n_tokens; }
    inline void set_token(size_t idx, token_t token)
    {
        if(idx < _batch.n_tokens && _batch.token)
            _batch.token[idx] = token;
    }
    inline void set_embd(float embd) { *_batch.embd = embd; }
    inline void set_pos(size_t idx, int32_t pos) { _batch.pos[idx] = pos; }
    inline void set_seq_id(size_t row, size_t col, seq_id_t val)
    {
        _batch.seq_id[row][col] = val;
    }
    inline void set_logits(int8_t logits) { *_batch.logits = logits; }

  private:
    llama_batch _batch;
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

    inline const llama_model *data() const { return _model; }

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

        const vocab_t *vocab = llama_model_get_vocab(_model);
        if(!vocab)
            return {};

        int n_tokens = llama_tokenize(vocab,
                                      prompt.c_str(),
                                      prompt.length(),
                                      nullptr,
                                      0,
                                      add_special,
                                      parse_special);
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
        if(n_tokens <= 0)
            return {};

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

    int32_t decode(batch &bt)
    {
        if(!_ctx)
            return -1;

        return llama_decode(_ctx, bt.data());
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
    sampler()
        : _smpl{llama_sampler_chain_init(llama_sampler_chain_default_params())}
    {
    }

    sampler(sampler_params_t params)
        : _smpl{llama_sampler_chain_init(params)}
    {
    }

    sampler(sampler_params_t params,
            int32_t          top_k,
            float            top_p,
            size_t           top_p_keep,
            float            min_p,
            size_t           min_p_keep,
            float            temp,
            uint32_t         seed)
        : _smpl{llama_sampler_chain_init(params)}
    {
        init_top_k(top_k);
        init_top_p(top_p, top_p_keep);
        init_min_p(min_p, min_p_keep);
        init_temp(temp);
        init_dist(seed);
    }

    ~sampler()
    {
        if(_smpl)
            llama_sampler_free(_smpl);
    }

    static sampler_params_t default_params()
    {
        return llama_sampler_chain_default_params();
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

    void init_temp(float temp)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_temp(temp));
    }

    void init_dist(uint32_t seed)
    {
        if(!_smpl)
            return;

        llama_sampler_chain_add(_smpl, llama_sampler_init_dist(seed));
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