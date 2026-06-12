#ifndef VECTOR_INDEX_HPP
#define VECTOR_INDEX_HPP

#include <algorithm>
#include <faiss/IndexFlat.h>
#include <faiss/index_io.h>
#include <faiss/impl/AuxIndexStructures.h>

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
        {
            delete _index;
        }
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

    void add(vindex_count_t vectors_num, const float *vectors)
    {
        _index->add(vectors_num, vectors);
    }

    void search(vindex_count_t vectors_num,
                const float   *vectors,
                vindex_count_t ret_num,
                float         *distances,
                vindex_idx_t  *indexs)
    {
        _index->search(vectors_num, vectors, ret_num, distances, indexs);
    }

    void range_search(vindex_count_t                vectors_num,
                      const float                  *vectors,
                      float                         radius,
                      vindex_range_search_result_t *result)
    {
        _index->range_search(vectors_num, vectors, radius, result);
    }

    void save(const char *filename) { faiss::write_index(_index, filename); }

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

    void reset() { _index->reset(); }

    vindex_count_t total() const { return _index->ntotal; }

  private:
    T *_index;
};

} // namespace hj

#endif // VECTOR_INDEX_HPP