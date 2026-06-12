#include <gtest/gtest.h>
#include <hj/ai/vector_index.hpp>

#include <filesystem>

TEST(vector_index, build)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    EXPECT_EQ(index.total(), 0u);
}

TEST(vector_index, add)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);
    EXPECT_EQ(index.total(), 0u);

    const float vectors[] = {
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
    };
    index.add(3, vectors);
    EXPECT_EQ(index.total(), 3u);
}

TEST(vector_index, search)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    const float vectors[] = {
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
    };
    index.add(3, vectors);

    const float  query[] = {1.0f, 0.0f};
    float        distances[2]{};
    faiss::idx_t indexs[2]{};

    index.search(1, query, 2, distances, indexs);

    EXPECT_EQ(indexs[0], 0);
    EXPECT_EQ(indexs[1], 2);
    EXPECT_FLOAT_EQ(distances[0], 0.0f);
    EXPECT_FLOAT_EQ(distances[1], 1.0f);
}

TEST(vector_index, range_search)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    const float vectors[] = {
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
    };
    const float query[] = {1.0f, 0.0f};

    index.add(3, vectors);

    hj::vindex_range_search_result_t result(1, true);
    index.range_search(1, query, 1.1f, &result);

    const size_t result_count = result.lims[1] - result.lims[0];
    ASSERT_EQ(result_count, 2u);

    const bool has_0 = std::find(result.labels, result.labels + result_count, 0)
                       != (result.labels + result_count);
    const bool has_2 = std::find(result.labels, result.labels + result_count, 2)
                       != (result.labels + result_count);

    EXPECT_TRUE(has_0);
    EXPECT_TRUE(has_2);
}

TEST(vector_index, empty_search)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);
    EXPECT_EQ(index.total(), 0u);

    const float  query[] = {0.0f, 0.0f};
    float        distances[1]{};
    faiss::idx_t indexs[1]{};

    // Searching in empty index should not crash
    // Results are undefined but should not cause errors
    index.search(1, query, 1, distances, indexs);
}

TEST(vector_index, multiple_queries)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    const float vectors[] = {
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
    };
    index.add(3, vectors);

    const float queries[] = {
        1.0f,
        0.0f, // query 1
        0.0f,
        1.0f, // query 2
    };
    float        distances[4]{}; // 2 queries * 2 results
    faiss::idx_t indexs[4]{};

    index.search(2, queries, 2, distances, indexs);

    // First query [1, 0] results
    EXPECT_EQ(indexs[0], 0); // [1, 0]
    EXPECT_EQ(indexs[1], 2); // [1, 1]

    // Second query [0, 1] results
    EXPECT_EQ(indexs[2], 1); // [0, 1]
    EXPECT_EQ(indexs[3], 2); // [1, 1]
}

TEST(vector_index, save)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    const float vectors[] = {
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
    };
    index.add(3, vectors);

    // for relative path
    {
        const std::string relative_filename = "./test_index.faissindex";
        if(std::filesystem::exists(relative_filename))
        {
            std::filesystem::remove(relative_filename);
        }

        index.save(relative_filename.c_str());
        EXPECT_TRUE(std::filesystem::exists(relative_filename));
    }

    // for abs path
    {
        const std::string abs_filename =
            std::filesystem::current_path().string() + "/"
            + "test_index.faissindex";
        if(std::filesystem::exists(abs_filename))
        {
            std::filesystem::remove(abs_filename);
        }

        index.save(abs_filename.c_str());
        EXPECT_TRUE(std::filesystem::exists(abs_filename));
    }
}

TEST(vector_index, load)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    const float vectors[] = {
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
    };
    index.add(3, vectors);

    // for relative path
    {
        std::string relative_filename = "./test_index_load.faissindex";
        if(std::filesystem::exists(relative_filename))
        {
            std::filesystem::remove(relative_filename);
        }
        index.save(relative_filename.c_str());

        hj::vector_index<hj::vindex_flat_l2_t> relative_index;
        EXPECT_TRUE(relative_index.load(relative_filename.c_str()));
        EXPECT_EQ(relative_index.total(), 3u);
        const float  query[] = {1.0f, 0.0f};
        float        distances[2]{};
        faiss::idx_t indexs[2]{};

        relative_index.search(1, query, 2, distances, indexs);

        EXPECT_EQ(indexs[0], 0);
        EXPECT_EQ(indexs[1], 2);
    }

    // Load the index and verify
    {
        std::string abs_filename = std::filesystem::current_path().string()
                                   + "/" + "test_index.faissindex";
        if(std::filesystem::exists(abs_filename))
        {
            std::filesystem::remove(abs_filename);
        }
        index.save(abs_filename.c_str());

        hj::vector_index<hj::vindex_flat_l2_t> abs_index;
        EXPECT_TRUE(abs_index.load(abs_filename.c_str()));
        EXPECT_EQ(abs_index.total(), 3u);
        const float  query[] = {1.0f, 0.0f};
        float        distances[2]{};
        faiss::idx_t indexs[2]{};

        abs_index.search(1, query, 2, distances, indexs);

        EXPECT_EQ(indexs[0], 0);
        EXPECT_EQ(indexs[1], 2);
    }
}

TEST(vector_index, reset)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    const float vectors[] = {
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
    };
    index.add(3, vectors);
    EXPECT_EQ(index.total(), 3u);

    index.reset();
    EXPECT_EQ(index.total(), 0u);
}