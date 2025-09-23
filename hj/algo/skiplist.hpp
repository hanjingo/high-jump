// for more information, see: 
// https://github.com/redis/redis/blob/unstable/src/t_zset.c

#ifndef SKIPLIST_HPP
#define SKIPLIST_HPP

#include <random>
#include <vector>
#include <functional>
#include <memory>
#include <cassert>

#define SKIPLIST_MAXLEVEL 32

#define SKIPLIST_P 0.25

namespace hj {

template<typename T>
struct _skiplist_node 
{
    T obj;                               // Data object
    double score;                        // Score for ordering
    struct _skiplist_node* backward;     // Backward pointer

    struct _skiplist_level 
    {
        struct _skiplist_node* forward;  // Forward pointer
        unsigned long span;              // Span to next node
    } level_[];                          // Flexible array for levels

    _skiplist_node() = delete;
    _skiplist_node(int level_, double score, const T& obj) 
        : obj(obj)
        , score(score)
        , backward(nullptr) 
    {}
};

template<typename T>
class skiplist 
{
public:
    using node_t     = _skiplist_node<T>;
    using compare_fn = std::function<bool(const T&, const T&)>;

private:
    struct _skiplist_level 
    {
        node_t* forward;
        unsigned long span;
    };

    node_t* header_;       // Header node (sentinel)
    node_t* tail_;         // Tail pointer for O(1) access to last element
    unsigned long length_; // Number of elements
    int level_;            // Current maximum level

    // Random number generator for level_ generation
    mutable std::random_device rd_;
    mutable std::mt19937 gen_;
    mutable std::uniform_real_distribution<double> dis_;
    compare_fn compare_;

public:
    class iterator 
    {
    private:
        node_t* current;

    public:
        iterator(node_t* node) : current(node) {}

        T& operator*() { return current->obj; }
        const T& operator*() const { return current->obj; }

        T* operator->() { return &current->obj; }
        const T* operator->() const { return &current->obj; }

        iterator& operator++() 
        {
            if (current) current = current->level_[0].forward;
            return *this;
        }

        iterator operator++(int) 
        {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        iterator& operator--() 
        {
            if (current) 
                current = current->backward;
            return *this;
        }

        iterator operator--(int) 
        {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return current != other.current; }

        inline double score() const { return current ? current->score : 0.0; }
        inline node_t* node() const { return current; }
    };

    class const_iterator 
    {
    private:
        const node_t* current;

    public:
        const_iterator(const node_t* node) : current(node) {}

        // Convert from iterator to const_iterator
        const_iterator(const iterator& it) : current(it.node()) {}

        const T& operator*() const { return current->obj; }
        const T* operator->() const { return &current->obj; }

        const_iterator& operator++() 
        {
            if (current) current = current->level_[0].forward;
            return *this;
        }

        const_iterator operator++(int) 
        {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        const_iterator& operator--() 
        {
            if (current) 
                current = current->backward;
            return *this;
        }

        const_iterator operator--(int) 
        {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const const_iterator& other) const { return current == other.current; }
        bool operator!=(const const_iterator& other) const { return current != other.current; }

        // Allow comparison between iterator and const_iterator
        bool operator==(const iterator& other) const { return current == other.node(); }
        bool operator!=(const iterator& other) const { return current != other.node(); }

        inline double score() const { return current ? current->score : 0.0; }
        inline const node_t* node() const { return current; }
    };

public:
    explicit skiplist(compare_fn cmp = std::less<T>()) 
        : tail_(nullptr)
        , length_(0)
        , level_(1)
        , gen_(rd_())
        , dis_(0.0, 1.0)
        , compare_(cmp) 
    {
        header_ = create_node_(SKIPLIST_MAXLEVEL, 0, T{});
        for (int j = 0; j < SKIPLIST_MAXLEVEL; j++) 
        {
            header_->level_[j].forward = nullptr;
            header_->level_[j].span = 0;
        }
        header_->backward = nullptr;
    }

    ~skiplist() 
    {
        release();
    }

    inline unsigned long size() const { return length_; }
    inline bool empty() const { return length_ == 0; }
    inline node_t* first() const { return header_->level_[0].forward; }
    inline node_t* last() const { return tail_; }
    inline iterator begin() { return iterator(first()); }
    inline iterator end() { return iterator(nullptr); }
    inline const_iterator begin() const { return const_iterator(first()); }
    inline const_iterator end() const { return const_iterator(nullptr); }
    inline const_iterator cbegin() const { return const_iterator(first()); }
    inline const_iterator cend() const { return const_iterator(nullptr); }

    // Insert element with score
    node_t* insert(double score, const T& obj) 
    {
        node_t* update[SKIPLIST_MAXLEVEL];
        unsigned long rank[SKIPLIST_MAXLEVEL];
        node_t* x = header_;

        // Search for insertion point and build update array
        for (int i = level_ - 1; i >= 0; i--) 
        {
            rank[i] = (i == (level_ - 1)) ? 0 : rank[i + 1];
            while (x->level_[i].forward &&
                   (x->level_[i].forward->score < score ||
                    (x->level_[i].forward->score == score && compare_(x->level_[i].forward->obj, obj)))) 
            {
                rank[i] += x->level_[i].span;
                x = x->level_[i].forward;
            }
            update[i] = x;
        }

        // Generate random level_ for new node
        int new_level = random_level_();
        if (new_level > level_) 
        {
            for (int i = level_; i < new_level; i++) 
            {
                rank[i] = 0;
                update[i] = header_;
                update[i]->level_[i].span = length_;
            }
            level_ = new_level;
        }

        // Create and insert new node
        x = create_node_(new_level, score, obj);
        for (int i = 0; i < new_level; i++) 
        {
            x->level_[i].forward = update[i]->level_[i].forward;
            update[i]->level_[i].forward = x;

            // Update span
            x->level_[i].span = update[i]->level_[i].span - (rank[0] - rank[i]);
            update[i]->level_[i].span = (rank[0] - rank[i]) + 1;
        }

        // Increment span for untouched levels
        for (int i = new_level; i < level_; i++)
            update[i]->level_[i].span++;

        // Set backward pointer
        x->backward = (update[0] == header_) ? nullptr : update[0];
        if (x->level_[0].forward)
            x->level_[0].forward->backward = x;
        else
            tail_ = x;

        length_++;
        return x;
    }

    // Delete element by score and object
    bool delete_node(double score, const T& obj) 
    {
        node_t* update[SKIPLIST_MAXLEVEL];
        node_t* x = header_;

        // Search for deletion point
        for (int i = level_ - 1; i >= 0; i--) 
        {
            while (x->level_[i].forward &&
                   (x->level_[i].forward->score < score ||
                    (x->level_[i].forward->score == score && compare_(x->level_[i].forward->obj, obj)))) 
            {
                x = x->level_[i].forward;
            }
            update[i] = x;
        }

        x = x->level_[0].forward;
        if (x && x->score == score && !compare_(x->obj, obj) && !compare_(obj, x->obj)) 
        {
            delete_node_internal_(x, update);
            return true;
        }

        return false;
    }

    // Get element by rank (0-based)
    node_t* get_element_by_rank(unsigned long rank) const 
    {
        if (rank >= length_) 
            return nullptr;

        node_t* x = header_;
        unsigned long traversed = 0;
        
        for (int i = level_ - 1; i >= 0; i--) 
        {
            while (x->level_[i].forward && 
                   (traversed + x->level_[i].span - 1) < rank)
            {
                traversed += x->level_[i].span;
                x = x->level_[i].forward;
            }
        }
        
        x = x->level_[0].forward;
        return x;
    }

    // Get rank of element (0-based)
    unsigned long get_rank(double score, const T& obj) const 
    {
        node_t* x = header_;
        unsigned long rank = 0;
        for (int i = level_ - 1; i >= 0; i--) 
        {
            while (x->level_[i].forward &&
                   (x->level_[i].forward->score < score ||
                    (x->level_[i].forward->score == score && compare_(x->level_[i].forward->obj, obj)))) 
            {
                rank += x->level_[i].span;
                x = x->level_[i].forward;
            }

            if (x->level_[i].forward && x->level_[i].forward->score == score && 
                !compare_(x->level_[i].forward->obj, obj) && !compare_(obj, x->level_[i].forward->obj)) 
            {
                rank += x->level_[i].span;
                return rank - 1;  // Convert to 0-based
            }
        }

        return length_;  // Not found
    }

    // Find first node in range [min_score, max_score]
    node_t* first_in_range(double min_score, double max_score) const 
    {
        if (!is_in_range_(min_score, max_score)) 
            return nullptr;

        node_t* x = header_;
        for (int i = level_ - 1; i >= 0; i--) 
        {
            while (x->level_[i].forward && x->level_[i].forward->score < min_score)
                x = x->level_[i].forward;
        }

        x = x->level_[0].forward;
        if (!x || x->score > max_score) 
            return nullptr;

        return x;
    }

    // Find last node in range [min_score, max_score]
    node_t* last_in_range(double min_score, double max_score) const 
    {
        if (!is_in_range_(min_score, max_score)) 
            return nullptr;

        node_t* x = header_;
        for (int i = level_ - 1; i >= 0; i--) 
        {
            while (x->level_[i].forward && x->level_[i].forward->score <= max_score)
                x = x->level_[i].forward;
        }

        if (!x || x->score < min_score) 
            return nullptr;

        return x;
    }

    // Delete nodes in score range
    unsigned long delete_range_by_score(double min_score, double max_score) 
    {
        node_t* update[SKIPLIST_MAXLEVEL];
        node_t* x = header_;
        unsigned long removed = 0;

        // Find start position
        for (int i = level_ - 1; i >= 0; i--) 
        {
            while (x->level_[i].forward && x->level_[i].forward->score < min_score)
                x = x->level_[i].forward;

            update[i] = x;
        }

        x = x->level_[0].forward;

        // Delete nodes in range
        while (x && x->score <= max_score) 
        {
            node_t* next = x->level_[0].forward;
            delete_node_internal_(x, update);
            removed++;
            x = next;
        }

        return removed;
    }

    // Delete nodes by rank range [start, end] (0-based, inclusive)
    unsigned long delete_range_by_rank(unsigned long start, unsigned long end) 
    {
        if (start > end || start >= length_) 
            return 0;

        if (end >= length_) 
            end = length_ - 1;

        node_t* update[SKIPLIST_MAXLEVEL];
        node_t* x = header_;
        unsigned long traversed = 0;
        unsigned long removed = 0;

        // Find start position
        for (int i = level_ - 1; i >= 0; i--) 
        {
            while (x->level_[i].forward && traversed + x->level_[i].span <= start) 
            {
                traversed += x->level_[i].span;
                x = x->level_[i].forward;
            }
            update[i] = x;
        }

        x = x->level_[0].forward;
        unsigned long current_rank = start;
        
        // Delete nodes in range
        while (x && current_rank <= end) 
        {
            node_t* next = x->level_[0].forward;
            delete_node_internal_(x, update);
            removed++;
            current_rank++;
            x = next;
        }

        return removed;
    }

    // Validate skiplist structure
    bool validate() const 
    {
        node_t* x = header_;
        unsigned long count = 0;

        // Check level_ 0 (all nodes)
        x = header_->level_[0].forward;
        while (x) 
        {
            count++;
            if (x->level_[0].forward && x->score > x->level_[0].forward->score)
                return false;  // Score order violation

            x = x->level_[0].forward;
        }

        return count == length_;
    }

private:
    node_t* create_node_(int level_, double score, const T& obj) 
    {
        size_t size = sizeof(node_t) + level_ * sizeof(typename node_t::_skiplist_level);
        node_t* node = static_cast<node_t*>(std::malloc(size));
        new(node) node_t(level_, score, obj);
        return node;
    }

    void free_node_(node_t* node) 
    {
        node->~_skiplist_node();
        std::free(node);
    }

    int random_level_() const 
    {
        int level_ = 1;
        while (dis_(gen_) < SKIPLIST_P && level_ < SKIPLIST_MAXLEVEL)
            level_++;

        return level_;
    }

    void delete_node_internal_(node_t* x, node_t** update) 
    {
        // Update forward pointers and spans
        for (int i = 0; i < level_; i++) 
        {
            if (update[i]->level_[i].forward == x) 
            {
                update[i]->level_[i].span += x->level_[i].span - 1;
                update[i]->level_[i].forward = x->level_[i].forward;
            } 
            else 
            {
                update[i]->level_[i].span -= 1;
            }
        }

        // Update backward pointer
        if (x->level_[0].forward)
            x->level_[0].forward->backward = x->backward;
        else
            tail_ = x->backward;

        // Free node and update level_ if necessary
        free_node_(x);
        while (level_ > 1 && header_->level_[level_ - 1].forward == nullptr)
            level_--;

        length_--;
    }

    bool is_in_range_(double min_score, double max_score) const 
    {
        if (min_score > max_score) 
            return false;
        
        node_t* x = tail_;
        if (x == nullptr || x->score < min_score) 
            return false;

        x = header_->level_[0].forward;
        if (x == nullptr || x->score > max_score) 
            return false;

        return true;
    }

    void release() 
    {
        node_t* node = header_->level_[0].forward;
        while (node) 
        {
            node_t* next = node->level_[0].forward;
            free_node_(node);
            node = next;
        }
        free_node_(header_);
    }
};

// Range result for range queries
template<typename T>
struct range_result 
{
    std::vector<typename skiplist<T>::node_t*> nodes;
    unsigned long total_in_range;

    range_result() : total_in_range(0) {}
};

// Get nodes in score range with limit and offset
template<typename T>
range_result<T> get_range_by_score(const skiplist<T>& sl, 
                                   const double min_score, 
                                   const double max_score,
                                   const unsigned long offset = 0, 
                                   const long limit = -1) 
{
    range_result<T> result;
    
    auto* x = sl.first_in_range(min_score, max_score);
    if (!x) 
        return result;

    // Skip offset nodes
    for (unsigned long i = 0; i < offset && x && x->score <= max_score; i++) 
    {
        x = x->level_[0].forward;
        result.total_in_range++;
    }

    // Collect nodes up to limit
    long collected = 0;
    while (x && x->score <= max_score && (limit == -1 || collected < limit)) 
    {
        result.nodes.push_back(x);
        result.total_in_range++;
        collected++;
        x = x->level_[0].forward;
    }

    // Count remaining nodes in range
    while (x && x->score <= max_score) 
    {
        result.total_in_range++;
        x = x->level_[0].forward;
    }

    return result;
}

// Get nodes by rank range with limit and offset
template<typename T>
range_result<T> get_range_by_rank(const skiplist<T>& sl,
                                  const unsigned long start, 
                                  const unsigned long end_param,
                                  const bool reverse = false) 
{
    range_result<T> result;
    
    if (start > end_param || start >= sl.size()) 
        return result;

    unsigned long end = end_param;
    if (end >= sl.size()) 
        end = sl.size() - 1;

    if (!reverse) 
    {
        // Forward iteration
        for (unsigned long i = start; i <= end; i++) 
        {
            auto* node = sl.get_element_by_rank(i);
            if (node) 
            {
                result.nodes.push_back(node);
                result.total_in_range++;
            }
        }
    } 
    else 
    {
        // Reverse iteration
        for (unsigned long i = end + 1; i > start; i--) 
        {
            auto* node = sl.get_element_by_rank(i - 1);
            if (node) 
            {
                result.nodes.push_back(node);
                result.total_in_range++;
            }
        }
    }

    return result;
}

} // namespace hj

#endif // SKIPLIST_HPP