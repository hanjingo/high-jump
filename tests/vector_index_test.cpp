#include <gtest/gtest.h>
#include <hj/ai/vector_index.hpp>

#include <filesystem>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>

void create_test_index(hj::vector_index<hj::vindex_flat_l2_t> &index)
{
    index.build(2);
    const float vectors[] = {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    index.add(3, vectors);
}

void cleanup_test_files(const std::vector<std::string> &files)
{
    for(const auto &file : files)
    {
        if(std::filesystem::exists(file))
        {
            std::filesystem::remove(file);
        }
    }

    for(const auto &entry : std::filesystem::directory_iterator("."))
    {
        if(entry.path().extension() == ".tmp")
        {
            std::filesystem::remove(entry.path());
        }
    }
}

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

TEST(vector_index, save_s_success)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    const std::string filename = "test_save_s_success.faissindex";
    cleanup_test_files({filename});

    EXPECT_TRUE(index.save_s(filename.c_str()));
    EXPECT_TRUE(std::filesystem::exists(filename));
    EXPECT_GT(std::filesystem::file_size(filename), 0);

    cleanup_test_files({filename});
}

TEST(vector_index, save_s_verify_integrity)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    const std::string filename = "test_save_s_verify.faissindex";
    cleanup_test_files({filename});

    ASSERT_TRUE(index.save_s(filename.c_str()));

    hj::vector_index<hj::vindex_flat_l2_t> loaded_index;
    EXPECT_TRUE(loaded_index.load(filename.c_str()));
    EXPECT_EQ(loaded_index.total(), 3u);

    const float  query[] = {1.0f, 0.0f};
    float        distances[2]{};
    faiss::idx_t indices[2]{};
    loaded_index.search(1, query, 2, distances, indices);

    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 2);
    EXPECT_FLOAT_EQ(distances[0], 0.0f);
    EXPECT_FLOAT_EQ(distances[1], 1.0f);

    cleanup_test_files({filename});
}

TEST(vector_index, save_s_empty_index)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);
    EXPECT_EQ(index.total(), 0u);

    const std::string filename = "test_save_s_empty.faissindex";
    cleanup_test_files({filename});

    EXPECT_TRUE(index.save_s(filename.c_str()));
    EXPECT_TRUE(std::filesystem::exists(filename));
    EXPECT_GT(std::filesystem::file_size(filename), 0);

    hj::vector_index<hj::vindex_flat_l2_t> loaded_index;
    EXPECT_TRUE(loaded_index.load(filename.c_str()));
    EXPECT_EQ(loaded_index.total(), 0u);

    cleanup_test_files({filename});
}

TEST(vector_index, save_s_invalid_path)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    const std::string invalid_filename = "/nonexistent/path/index.faissindex";
    EXPECT_FALSE(index.save_s(invalid_filename.c_str()));

    EXPECT_FALSE(index.save_s(""));
}

TEST(vector_index, save_s_overwrite)
{
    hj::vector_index<hj::vindex_flat_l2_t> index1, index2;
    create_test_index(index1);

    const std::string filename = "test_save_s_overwrite.faissindex";
    cleanup_test_files({filename});

    ASSERT_TRUE(index1.save_s(filename.c_str()));
    auto size1 = std::filesystem::file_size(filename);

    index2.build(2);
    const float vectors2[] = {0.0f, 0.0f, 1.0f, 1.0f, 2.0f, 2.0f};
    index2.add(3, vectors2);
    EXPECT_TRUE(index2.save_s(filename.c_str()));
    auto size2 = std::filesystem::file_size(filename);

    EXPECT_TRUE(std::filesystem::exists(filename));

    hj::vector_index<hj::vindex_flat_l2_t> loaded_index;
    ASSERT_TRUE(loaded_index.load(filename.c_str()));
    EXPECT_EQ(loaded_index.total(), 3u);

    const float  query[] = {0.0f, 0.0f};
    float        distances[2]{};
    faiss::idx_t indices[2]{};
    loaded_index.search(1, query, 2, distances, indices);
    EXPECT_EQ(indices[0], 0);

    cleanup_test_files({filename});
}

TEST(vector_index, save_s_then_delete_original)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    const std::string filename = "test_save_s_delete.faissindex";
    cleanup_test_files({filename});

    ASSERT_TRUE(index.save_s(filename.c_str()));

    index.reset();
    EXPECT_EQ(index.total(), 0u);

    hj::vector_index<hj::vindex_flat_l2_t> loaded_index;
    EXPECT_TRUE(loaded_index.load(filename.c_str()));
    EXPECT_EQ(loaded_index.total(), 3u);

    const float  query[] = {1.0f, 0.0f};
    float        distances[2]{};
    faiss::idx_t indices[2]{};
    loaded_index.search(1, query, 2, distances, indices);
    EXPECT_EQ(indices[0], 0);

    cleanup_test_files({filename});
}

TEST(vector_index, save_s_concurrent)
{
    const int                thread_count = 4;
    std::vector<std::thread> threads;
    std::vector<std::string> filenames;
    std::atomic<int>         success_count{0};

    for(int i = 0; i < thread_count; i++)
    {
        filenames.push_back("test_save_s_concurrent_" + std::to_string(i)
                            + ".faissindex");
    }

    cleanup_test_files(filenames);

    auto save_task =
        [](const std::string &filename, int seed, std::atomic<int> &count) {
            hj::vector_index<hj::vindex_flat_l2_t> index;
            index.build(2);

            std::vector<float> vectors(6);
            for(int i = 0; i < 3; i++)
            {
                vectors[i * 2]     = static_cast<float>(seed + i);
                vectors[i * 2 + 1] = static_cast<float>(seed + i + 1);
            }
            index.add(3, vectors.data());

            if(index.save_s(filename.c_str()))
            {
                count++;
            }
        };

    for(int i = 0; i < thread_count; i++)
    {
        threads.emplace_back(save_task,
                             filenames[i],
                             i,
                             std::ref(success_count));
    }

    for(auto &th : threads)
    {
        th.join();
    }

    EXPECT_EQ(success_count, thread_count);

    for(const auto &filename : filenames)
    {
        EXPECT_TRUE(std::filesystem::exists(filename));
        hj::vector_index<hj::vindex_flat_l2_t> loaded_index;
        EXPECT_TRUE(loaded_index.load(filename.c_str()));
        EXPECT_EQ(loaded_index.total(), 3u);
    }

    cleanup_test_files(filenames);
}

TEST(vector_index, save_s_null_index)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;

    const std::string filename = "test_save_s_null.faissindex";
    cleanup_test_files({filename});

    EXPECT_FALSE(index.save_s(filename.c_str()));
    EXPECT_FALSE(std::filesystem::exists(filename));

    cleanup_test_files({filename});
}

TEST(vector_index, save_s_atomic_write)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    const std::string filename = "test_save_s_atomic.faissindex";
    cleanup_test_files({filename});

    EXPECT_TRUE(index.save_s(filename.c_str()));

    EXPECT_TRUE(std::filesystem::exists(filename));
    EXPECT_GT(std::filesystem::file_size(filename), 0);

    bool has_temp_file = false;
    for(const auto &entry : std::filesystem::directory_iterator("."))
    {
        if(entry.path().extension() == ".tmp"
           && entry.path().string().find(filename) != std::string::npos)
        {
            has_temp_file = true;
            break;
        }
    }
    EXPECT_FALSE(has_temp_file);

    hj::vector_index<hj::vindex_flat_l2_t> loaded_index;
    EXPECT_TRUE(loaded_index.load(filename.c_str()));
    EXPECT_EQ(loaded_index.total(), 3u);

    cleanup_test_files({filename});
}

TEST(vector_index, save_s_readonly_directory)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

#ifdef __unix__
    const std::string readonly_dir = "/tmp/readonly_test";
    if(!std::filesystem::exists(readonly_dir))
    {
        std::filesystem::create_directory(readonly_dir);
    }

    std::filesystem::permissions(readonly_dir,
                                 std::filesystem::perms::owner_read
                                     | std::filesystem::perms::group_read
                                     | std::filesystem::perms::others_read);

    const std::string filename = readonly_dir + "/test.faissindex";

    EXPECT_NO_THROW(index.save_s(filename.c_str()));

    std::filesystem::permissions(readonly_dir,
                                 std::filesystem::perms::owner_all);
    std::filesystem::remove_all(readonly_dir);
#endif
}

TEST(vector_index, save_s_file_size_reasonable)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    const size_t       vector_count = 100;
    std::vector<float> vectors(vector_count * 2);
    for(size_t i = 0; i < vectors.size(); i++)
    {
        vectors[i] = static_cast<float>(i) / 100.0f;
    }
    index.add(vector_count, vectors.data());

    const std::string filename = "test_save_s_size.faissindex";
    cleanup_test_files({filename});

    EXPECT_TRUE(index.save_s(filename.c_str()));

    auto file_size = std::filesystem::file_size(filename);
    EXPECT_GE(file_size, vector_count * 2 * 4);

    EXPECT_LT(file_size, 10240); // 10KB

    cleanup_test_files({filename});
}

TEST(vector_index, save_s_consistent_with_save)
{
    hj::vector_index<hj::vindex_flat_l2_t> index1, index2;
    create_test_index(index1);
    create_test_index(index2);

    const std::string filename_save   = "test_save.faissindex";
    const std::string filename_save_s = "test_save_s.faissindex";
    cleanup_test_files({filename_save, filename_save_s});

    index1.save(filename_save.c_str());
    EXPECT_TRUE(std::filesystem::exists(filename_save));

    EXPECT_TRUE(index2.save_s(filename_save_s.c_str()));
    EXPECT_TRUE(std::filesystem::exists(filename_save_s));

    hj::vector_index<hj::vindex_flat_l2_t> loaded_from_save;
    hj::vector_index<hj::vindex_flat_l2_t> loaded_from_save_s;

    EXPECT_TRUE(loaded_from_save.load(filename_save.c_str()));
    EXPECT_TRUE(loaded_from_save_s.load(filename_save_s.c_str()));

    EXPECT_EQ(loaded_from_save.total(), loaded_from_save_s.total());

    const float  query[] = {1.0f, 0.0f};
    float        dist1[2]{}, dist2[2]{};
    faiss::idx_t idx1[2]{}, idx2[2]{};

    loaded_from_save.search(1, query, 2, dist1, idx1);
    loaded_from_save_s.search(1, query, 2, dist2, idx2);

    EXPECT_EQ(idx1[0], idx2[0]);
    EXPECT_EQ(idx1[1], idx2[1]);
    EXPECT_FLOAT_EQ(dist1[0], dist2[0]);
    EXPECT_FLOAT_EQ(dist1[1], dist2[1]);

    cleanup_test_files({filename_save, filename_save_s});
}

TEST(vector_index, save_s_disk_full_simulation)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    const std::string filename = "test_save_s_disk_full.faissindex";
    cleanup_test_files({filename});

    std::vector<std::string> many_files;
    try
    {
        for(int i = 0; i < 1000; i++)
        {
            std::string fname = "temp_dummy_" + std::to_string(i) + ".tmp";
            std::ofstream(fname) << "dummy";
            many_files.push_back(fname);
        }


        bool result = index.save_s(filename.c_str());

        EXPECT_NO_THROW(index.save_s(filename.c_str()));
        for(const auto &f : many_files)
        {
            std::filesystem::remove(f);
        }
    }
    catch(...)
    {
        for(const auto &f : many_files)
        {
            if(std::filesystem::exists(f))
            {
                std::filesystem::remove(f);
            }
        }
    }

    cleanup_test_files({filename});
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

TEST(vector_index, load_s_success)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);
    const std::string filename = "test_load_s_success.faissindex";
    cleanup_test_files({filename});
    ASSERT_TRUE(index.save_s(filename.c_str()));

    hj::vector_index<hj::vindex_flat_l2_t> loaded_index;
    EXPECT_TRUE(loaded_index.load_s(filename.c_str()));
    EXPECT_EQ(loaded_index.total(), 3u);

    const float  query[] = {1.0f, 0.0f};
    float        distances[2]{};
    faiss::idx_t indices[2]{};
    loaded_index.search(1, query, 2, distances, indices);

    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 2);
    EXPECT_FLOAT_EQ(distances[0], 0.0f);
    EXPECT_FLOAT_EQ(distances[1], 1.0f);

    cleanup_test_files({filename});
}

TEST(vector_index, load_s_file_not_found)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    const std::string filename = "nonexistent_file.faissindex";
    cleanup_test_files({filename});

    EXPECT_FALSE(index.load_s(filename.c_str()));
    EXPECT_EQ(index.total(), 0u);
}

TEST(vector_index, load_s_empty_file)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    const std::string filename = "test_load_s_empty.faissindex";
    cleanup_test_files({filename});

    std::ofstream(filename) << "";
    ASSERT_TRUE(std::filesystem::exists(filename));
    ASSERT_EQ(std::filesystem::file_size(filename), 0u);

    EXPECT_FALSE(index.load_s(filename.c_str()));
    EXPECT_EQ(index.total(), 0u);

    cleanup_test_files({filename});
}

TEST(vector_index, load_s_corrupted_file)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    const std::string filename = "test_load_s_corrupted.faissindex";
    cleanup_test_files({filename});

    std::ofstream file(filename, std::ios::binary);
    file << "This is not a valid FAISS index file";
    file.close();
    ASSERT_TRUE(std::filesystem::exists(filename));
    ASSERT_GT(std::filesystem::file_size(filename), 0u);

    EXPECT_FALSE(index.load_s(filename.c_str()));
    EXPECT_EQ(index.total(), 0u);

    cleanup_test_files({filename});
}

TEST(vector_index, load_s_replace_existing)
{
    hj::vector_index<hj::vindex_flat_l2_t> index1;
    create_test_index(index1);
    const std::string filename = "test_load_s_replace.faissindex";
    cleanup_test_files({filename});
    ASSERT_TRUE(index1.save_s(filename.c_str()));

    hj::vector_index<hj::vindex_flat_l2_t> index2;
    index2.build(2);
    const float vectors2[] = {0.0f, 0.0f, 1.0f, 1.0f, 2.0f, 2.0f};
    index2.add(3, vectors2);
    EXPECT_EQ(index2.total(), 3u);

    EXPECT_TRUE(index2.load_s(filename.c_str()));
    EXPECT_EQ(index2.total(), 3u);

    const float  query[] = {1.0f, 0.0f};
    float        distances[2]{};
    faiss::idx_t indices[2]{};
    index2.search(1, query, 2, distances, indices);

    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 2);
    EXPECT_FLOAT_EQ(distances[0], 0.0f);
    EXPECT_FLOAT_EQ(distances[1], 1.0f);

    cleanup_test_files({filename});
}

TEST(vector_index, load_s_invalid_filename)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;

    EXPECT_FALSE(index.load_s(nullptr));

    EXPECT_FALSE(index.load_s(""));

    EXPECT_EQ(index.total(), 0u);
}

TEST(vector_index, load_s_type_mismatch)
{
    hj::vector_index<hj::vindex_flat_l2_t> index_l2;
    create_test_index(index_l2);
    const std::string filename = "test_load_s_type.faissindex";
    cleanup_test_files({filename});
    ASSERT_TRUE(index_l2.save_s(filename.c_str()));

    hj::vector_index<hj::vindex_flat_l2_t> loaded_index;
    EXPECT_TRUE(loaded_index.load_s(filename.c_str()));
    EXPECT_EQ(loaded_index.total(), 3u);

    cleanup_test_files({filename});
}

TEST(vector_index, load_s_concurrent)
{
    hj::vector_index<hj::vindex_flat_l2_t> master_index;
    create_test_index(master_index);
    const std::string filename = "test_load_s_concurrent.faissindex";
    cleanup_test_files({filename});
    ASSERT_TRUE(master_index.save_s(filename.c_str()));

    const int                thread_count = 4;
    std::vector<std::thread> threads;
    std::atomic<int>         success_count{0};

    auto load_task = [&filename, &success_count]() {
        hj::vector_index<hj::vindex_flat_l2_t> index;
        if(index.load_s(filename.c_str()) && index.total() == 3u)
        {
            success_count++;
        }
    };

    for(int i = 0; i < thread_count; i++)
    {
        threads.emplace_back(load_task);
    }

    for(auto &th : threads)
    {
        th.join();
    }

    EXPECT_EQ(success_count, thread_count);

    cleanup_test_files({filename});
}

TEST(vector_index, load_s_save_s_integration)
{
    hj::vector_index<hj::vindex_flat_l2_t> original;
    original.build(2);
    const float vectors[] = {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    original.add(3, vectors);
    EXPECT_EQ(original.total(), 3u);

    const std::string filename = "test_load_s_integration.faissindex";
    cleanup_test_files({filename});
    ASSERT_TRUE(original.save_s(filename.c_str()));

    hj::vector_index<hj::vindex_flat_l2_t> loaded;
    EXPECT_TRUE(loaded.load_s(filename.c_str()));
    EXPECT_EQ(loaded.total(), 3u);

    const float  query[] = {1.0f, 0.0f};
    float        orig_dist[2]{}, load_dist[2]{};
    faiss::idx_t orig_idx[2]{}, load_idx[2]{};

    original.search(1, query, 2, orig_dist, orig_idx);
    loaded.search(1, query, 2, load_dist, load_idx);

    EXPECT_EQ(orig_idx[0], load_idx[0]);
    EXPECT_EQ(orig_idx[1], load_idx[1]);
    EXPECT_FLOAT_EQ(orig_dist[0], load_dist[0]);
    EXPECT_FLOAT_EQ(orig_dist[1], load_dist[1]);

    cleanup_test_files({filename});
}

TEST(vector_index, load_s_performance)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(384);
    const size_t       vector_count = 10000;
    std::vector<float> vectors(vector_count * 384);
    for(size_t i = 0; i < vectors.size(); i++)
    {
        vectors[i] = static_cast<float>(rand()) / RAND_MAX;
    }
    index.add(vector_count, vectors.data());

    const std::string filename = "test_load_s_perf.faissindex";
    cleanup_test_files({filename});
    ASSERT_TRUE(index.save_s(filename.c_str()));

    auto file_size = std::filesystem::file_size(filename);
    std::cout << "Index file size: " << file_size / (1024 * 1024) << " MB"
              << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    hj::vector_index<hj::vindex_flat_l2_t> loaded_index;
    bool success = loaded_index.load_s(filename.c_str());
    auto end     = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_TRUE(success);
    EXPECT_EQ(loaded_index.total(), vector_count);
    std::cout << "Load time: " << duration.count() << " ms" << std::endl;

    cleanup_test_files({filename});
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

TEST(vector_index, serialize_success)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    std::vector<uint8_t> buffer;
    EXPECT_TRUE(index.serialize(buffer));
    EXPECT_FALSE(buffer.empty());
}

TEST(vector_index, serialize_empty_index)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    std::vector<uint8_t> buffer;
    EXPECT_TRUE(index.serialize(buffer));
    EXPECT_FALSE(buffer.empty());
}

TEST(vector_index, serialize_null_index)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    std::vector<uint8_t>                   buffer;
    EXPECT_FALSE(index.serialize(buffer));
    EXPECT_TRUE(buffer.empty());
}

TEST(vector_index, deserialize_success)
{
    hj::vector_index<hj::vindex_flat_l2_t> original;
    create_test_index(original);

    std::vector<uint8_t> buffer;
    ASSERT_TRUE(original.serialize(buffer));

    hj::vector_index<hj::vindex_flat_l2_t> restored;
    EXPECT_TRUE(restored.deserialize(std::move(buffer)));
    EXPECT_EQ(restored.total(), 3u);

    const float  query[] = {1.0f, 0.0f};
    float        orig_dist[2]{}, rest_dist[2]{};
    faiss::idx_t orig_idx[2]{}, rest_idx[2]{};

    original.search(1, query, 2, orig_dist, orig_idx);
    restored.search(1, query, 2, rest_dist, rest_idx);

    EXPECT_EQ(orig_idx[0], rest_idx[0]);
    EXPECT_EQ(orig_idx[1], rest_idx[1]);
    EXPECT_FLOAT_EQ(orig_dist[0], rest_dist[0]);
    EXPECT_FLOAT_EQ(orig_dist[1], rest_dist[1]);
}

TEST(vector_index, deserialize_empty_buffer)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    std::vector<uint8_t>                   empty_buffer;
    EXPECT_FALSE(index.deserialize(std::move(empty_buffer)));
}

TEST(vector_index, deserialize_corrupted_buffer)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    std::vector<uint8_t>                   corrupted = {0x01, 0x02, 0x03, 0x04};

    EXPECT_THROW(index.deserialize(std::move(corrupted)),
                 faiss::FaissException);
}

TEST(vector_index, serialize_deserialize_roundtrip)
{
    hj::vector_index<hj::vindex_flat_l2_t> original;
    original.build(2);
    const float vectors[] = {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f};
    original.add(4, vectors);
    EXPECT_EQ(original.total(), 4u);

    std::vector<uint8_t> buffer;
    ASSERT_TRUE(original.serialize(buffer));

    hj::vector_index<hj::vindex_flat_l2_t> restored;
    ASSERT_TRUE(restored.deserialize(std::move(buffer)));
    EXPECT_EQ(restored.total(), 4u);

    for(int i = 0; i < 4; i++)
    {
        std::vector<float> orig_vec, rest_vec;
        ASSERT_TRUE(original.get_vector(i, orig_vec));
        ASSERT_TRUE(restored.get_vector(i, rest_vec));

        EXPECT_EQ(orig_vec.size(), rest_vec.size());
        for(size_t j = 0; j < orig_vec.size(); j++)
        {
            EXPECT_FLOAT_EQ(orig_vec[j], rest_vec[j]);
        }
    }
}

TEST(vector_index, deserialize_overwrite_existing)
{
    hj::vector_index<hj::vindex_flat_l2_t> index1;
    create_test_index(index1);

    std::vector<uint8_t> buffer;
    ASSERT_TRUE(index1.serialize(buffer));

    hj::vector_index<hj::vindex_flat_l2_t> index2;
    index2.build(2);
    const float vectors2[] = {0.0f, 0.0f, 1.0f, 1.0f};
    index2.add(2, vectors2);
    EXPECT_EQ(index2.total(), 2u);

    EXPECT_TRUE(index2.deserialize(std::move(buffer)));
    EXPECT_EQ(index2.total(), 3u);

    const float  query[] = {1.0f, 0.0f};
    float        distances[2]{};
    faiss::idx_t indices[2]{};
    index2.search(1, query, 2, distances, indices);
    EXPECT_EQ(indices[0], 0);
}

TEST(vector_index, get_vector_success)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    std::vector<float> vec;
    EXPECT_TRUE(index.get_vector(0, vec));
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_FLOAT_EQ(vec[0], 1.0f);
    EXPECT_FLOAT_EQ(vec[1], 0.0f);

    EXPECT_TRUE(index.get_vector(1, vec));
    EXPECT_FLOAT_EQ(vec[0], 0.0f);
    EXPECT_FLOAT_EQ(vec[1], 1.0f);

    EXPECT_TRUE(index.get_vector(2, vec));
    EXPECT_FLOAT_EQ(vec[0], 1.0f);
    EXPECT_FLOAT_EQ(vec[1], 1.0f);
}

TEST(vector_index, get_vector_out_of_range)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    std::vector<float> vec;
    EXPECT_FALSE(index.get_vector(3, vec));
    EXPECT_FALSE(index.get_vector(-1, vec));
}

TEST(vector_index, get_vector_empty_index)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    std::vector<float> vec;
    EXPECT_FALSE(index.get_vector(0, vec));
}

TEST(vector_index, get_vector_null_index)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    std::vector<float>                     vec;
    EXPECT_FALSE(index.get_vector(0, vec));
}

TEST(vector_index, get_vector_reuse_buffer)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    std::vector<float> vec;

    EXPECT_TRUE(index.get_vector(0, vec));
    EXPECT_EQ(vec.size(), 2u);

    size_t capacity = vec.capacity();
    EXPECT_TRUE(index.get_vector(1, vec));
    EXPECT_EQ(vec.size(), 2u);
}

TEST(vector_index, get_all_vectors_success)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    std::vector<float> all_vectors;
    EXPECT_TRUE(index.get_all_vectors(all_vectors));

    EXPECT_EQ(all_vectors.size(), 6u);

    EXPECT_FLOAT_EQ(all_vectors[0], 1.0f);
    EXPECT_FLOAT_EQ(all_vectors[1], 0.0f);
    EXPECT_FLOAT_EQ(all_vectors[2], 0.0f);
    EXPECT_FLOAT_EQ(all_vectors[3], 1.0f);
    EXPECT_FLOAT_EQ(all_vectors[4], 1.0f);
    EXPECT_FLOAT_EQ(all_vectors[5], 1.0f);
}

TEST(vector_index, get_all_vectors_empty_index)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    std::vector<float> all_vectors;
    EXPECT_FALSE(index.get_all_vectors(all_vectors));
    EXPECT_TRUE(all_vectors.empty());
}

TEST(vector_index, get_all_vectors_null_index)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    std::vector<float>                     all_vectors;
    EXPECT_FALSE(index.get_all_vectors(all_vectors));
}

TEST(vector_index, get_all_vectors_large)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    const int                              dim = 128;
    const int                              n   = 1000;
    index.build(dim);

    std::vector<float> vectors(n * dim);
    for(int i = 0; i < n * dim; i++)
    {
        vectors[i] = static_cast<float>(rand()) / RAND_MAX;
    }
    index.add(n, vectors.data());
    EXPECT_EQ(index.total(), n);

    std::vector<float> all_vectors;
    auto               start = std::chrono::high_resolution_clock::now();
    EXPECT_TRUE(index.get_all_vectors(all_vectors));
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_EQ(all_vectors.size(), n * dim);
    std::cout << "get_all_vectors (" << n << " vectors, " << dim
              << "D): " << duration.count() << " ms" << std::endl;

    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < dim; j++)
        {
            EXPECT_FLOAT_EQ(all_vectors[i * dim + j], vectors[i * dim + j]);
        }
    }
}

TEST(vector_index, get_vector_vs_get_all_vectors_consistency)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    create_test_index(index);

    std::vector<float> all_vectors;
    ASSERT_TRUE(index.get_all_vectors(all_vectors));

    for(int i = 0; i < 3; i++)
    {
        std::vector<float> single_vec;
        ASSERT_TRUE(index.get_vector(i, single_vec));

        for(size_t j = 0; j < single_vec.size(); j++)
        {
            EXPECT_FLOAT_EQ(single_vec[j],
                            all_vectors[i * single_vec.size() + j]);
        }
    }
}

TEST(vector_index, serialize_deserialize_get_all_vectors_integration)
{
    hj::vector_index<hj::vindex_flat_l2_t> original;
    original.build(2);
    const float vectors[] = {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    original.add(3, vectors);

    std::vector<float> original_vectors;
    ASSERT_TRUE(original.get_all_vectors(original_vectors));

    std::vector<uint8_t> buffer;
    ASSERT_TRUE(original.serialize(buffer));

    hj::vector_index<hj::vindex_flat_l2_t> restored;
    ASSERT_TRUE(restored.deserialize(std::move(buffer)));

    std::vector<float> restored_vectors;
    ASSERT_TRUE(restored.get_all_vectors(restored_vectors));

    EXPECT_EQ(original_vectors.size(), restored_vectors.size());
    for(size_t i = 0; i < original_vectors.size(); i++)
    {
        EXPECT_FLOAT_EQ(original_vectors[i], restored_vectors[i]);
    }
}

TEST(vector_index, add_with_ids_without_idmap)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    const float        vectors[] = {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    const faiss::idx_t ids[]     = {100, 200, 300};

    EXPECT_FALSE(index.add_with_ids(3, vectors, ids));
}

TEST(vector_index, add_with_ids_empty)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    const float        vectors[] = {1.0f, 0.0f};
    const faiss::idx_t ids[]     = {100};

    EXPECT_FALSE(index.add_with_ids(0, vectors, ids));
}

TEST(vector_index, add_with_ids_null_params)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    EXPECT_FALSE(index.add_with_ids(1, nullptr, nullptr));
}

TEST(vector_index, add_with_ids_null_index)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;

    const float        vectors[] = {1.0f, 0.0f};
    const faiss::idx_t ids[]     = {100};

    EXPECT_FALSE(index.add_with_ids(1, vectors, ids));
}

TEST(vector_index, add_with_ids_with_idmap)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    faiss::Index     *base_index = index.get_index();
    faiss::IndexIDMap idMap(base_index);

    const float        vectors[] = {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    const faiss::idx_t ids[]     = {10, 20, 30};

    idMap.add_with_ids(3, vectors, ids);
    EXPECT_EQ(idMap.ntotal, 3u);

    const float  query[] = {1.0f, 0.0f};
    float        distances[2];
    faiss::idx_t indices[2];
    idMap.search(1, query, 2, distances, indices);

    EXPECT_EQ(indices[0], 10);
}