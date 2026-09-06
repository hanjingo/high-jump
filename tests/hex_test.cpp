#include <gtest/gtest.h>
#include <hj/encoding/hex.hpp>
#include <array>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>

TEST(hex, decode)
{
    uint32_t n1 = hj::hex::decode<uint32_t>("0F");
    ASSERT_EQ(n1, 0xF);
    uint32_t n2 = hj::hex::decode<uint32_t>("FF");
    ASSERT_EQ(n2, 0xFF);
    uint32_t n3 = hj::hex::decode<uint32_t>("0FFF");
    ASSERT_EQ(n3, 0xFFF);

    EXPECT_THROW(hj::hex::decode<uint32_t>("F"), std::invalid_argument);
    EXPECT_THROW(hj::hex::decode<uint8_t>("GG"), std::invalid_argument);

    std::string str_decoded = hj::hex::decode<std::string>("48656C6C6F");
    ASSERT_EQ(str_decoded, "Hello");

    std::vector<uint8_t> vec_decoded =
        hj::hex::decode<std::vector<uint8_t>>("010203FF");
    ASSERT_EQ(vec_decoded.size(), 4);
    ASSERT_EQ(vec_decoded[0], 0x01);
    ASSERT_EQ(vec_decoded[3], 0xFF);

    std::istringstream in1("0F");
    std::istringstream in2("FF");
    std::istringstream in3("0FFF");
    std::ostringstream out1;
    std::ostringstream out2;
    std::ostringstream out3;

    EXPECT_TRUE(hj::hex::decode(out1, in1));
    auto res1 = out1.str();
    ASSERT_EQ(res1.size(), 1);
    ASSERT_EQ(static_cast<unsigned char>(res1[0]), 0x0F);

    EXPECT_TRUE(hj::hex::decode(out2, in2));
    auto res2 = out2.str();
    ASSERT_EQ(res2.size(), 1);
    ASSERT_EQ(static_cast<unsigned char>(res2[0]), 0xFF);

    EXPECT_TRUE(hj::hex::decode(out3, in3));
    auto res3 = out3.str();
    ASSERT_EQ(res3.size(), 2);
    ASSERT_EQ(static_cast<unsigned char>(res3[0]), 0x0F);
    ASSERT_EQ(static_cast<unsigned char>(res3[1]), 0xFF);

    std::istringstream in_odd("FFF");
    std::ostringstream out_odd;
    EXPECT_FALSE(hj::hex::decode(out_odd, in_odd));
}

TEST(hex, decode_unsigned_integrals)
{
    uint8_t u8 = hj::hex::decode<uint8_t>("FF");
    ASSERT_EQ(u8, 0xFF);

    uint16_t u16 = hj::hex::decode<uint16_t>("FFFF");
    ASSERT_EQ(u16, 0xFFFF);

    uint32_t u32 = hj::hex::decode<uint32_t>("12345678");
    ASSERT_EQ(u32, 0x12345678);

    auto opt_overflow_u8 = hj::hex::try_decode<uint8_t>("0100");
    ASSERT_FALSE(opt_overflow_u8.has_value());

    auto opt_too_long_u8 = hj::hex::try_decode<uint8_t>("00FF");
    ASSERT_FALSE(opt_too_long_u8.has_value());

    auto opt_overflow_u16 = hj::hex::try_decode<uint16_t>("010000");
    ASSERT_FALSE(opt_overflow_u16.has_value());
}

TEST(hex, encode)
{
    ASSERT_STREQ(hj::hex::encode(255, true).c_str(), "FF");
    ASSERT_STREQ(hj::hex::encode(255, false).c_str(), "ff");

    ASSERT_STREQ(hj::hex::encode(4095, true).c_str(), "0FFF");
    ASSERT_STREQ(hj::hex::encode(4095, false).c_str(), "0fff");

    ASSERT_EQ(hj::hex::encode("Hello", true), "48656C6C6F");

    std::vector<uint8_t> bytes = {0x00, 0x1A, 0x2B, 0xFF};
    ASSERT_EQ(hj::hex::encode(bytes, false), "001a2bff");

    std::array<uint8_t, 3> arr_bytes = {0x12, 0x34, 0x56};
    ASSERT_EQ(hj::hex::encode(arr_bytes, true), "123456");

    std::istringstream in1("\xFF", std::ios::binary);
    std::ostringstream out1;
    EXPECT_TRUE(hj::hex::encode(out1, in1, true));
    ASSERT_EQ(out1.str(), "FF");

    std::istringstream in2(std::string("\xFF\x0F", 2), std::ios::binary);
    std::ostringstream out2;
    EXPECT_TRUE(hj::hex::encode(out2, in2, true));
    ASSERT_EQ(out2.str(), "FF0F");
}

TEST(hex, is_valid)
{
    EXPECT_TRUE(hj::hex::is_valid("00"));
    EXPECT_TRUE(hj::hex::is_valid("0A"));
    EXPECT_TRUE(hj::hex::is_valid("ff"));
    EXPECT_TRUE(hj::hex::is_valid("ABCDEFabcdef0123456789"));

    EXPECT_FALSE(hj::hex::is_valid("0G"));
    EXPECT_FALSE(hj::hex::is_valid("xyz"));
    EXPECT_FALSE(hj::hex::is_valid("12 34"));
    EXPECT_FALSE(hj::hex::is_valid("12-34"));

    EXPECT_FALSE(hj::hex::is_valid("F"));
    EXPECT_FALSE(hj::hex::is_valid("123"));
    EXPECT_FALSE(hj::hex::is_valid(""));

    {
        std::string valid_hex   = "AABBCCDDEEFF";
        std::string invalid_hex = "AABBCCGG";
        std::string odd_hex     = "AABBCCD";
        std::string empty_hex   = "";

        {
            std::ofstream ofs("tmp_valid_hex.txt", std::ios::binary);
            ofs << valid_hex;
        }
        {
            std::ofstream ofs("tmp_invalid_hex.txt", std::ios::binary);
            ofs << invalid_hex;
        }
        {
            std::ofstream ofs("tmp_odd_hex.txt", std::ios::binary);
            ofs << odd_hex;
        }
        {
            std::ofstream ofs("tmp_empty_hex.txt", std::ios::binary);
            ofs << empty_hex;
        }

        EXPECT_TRUE(hj::hex::is_valid_file("tmp_valid_hex.txt"));
        EXPECT_FALSE(hj::hex::is_valid_file("tmp_invalid_hex.txt"));
        EXPECT_FALSE(hj::hex::is_valid_file("tmp_odd_hex.txt"));
        EXPECT_FALSE(hj::hex::is_valid_file("tmp_empty_hex.txt"));

        std::ifstream fin1("tmp_valid_hex.txt", std::ios::binary);
        EXPECT_TRUE(hj::hex::is_valid(fin1));
        fin1.close();

        std::ifstream fin2("tmp_invalid_hex.txt", std::ios::binary);
        EXPECT_FALSE(hj::hex::is_valid(fin2));
        fin2.close();

        std::ifstream fin3("tmp_odd_hex.txt", std::ios::binary);
        EXPECT_FALSE(hj::hex::is_valid(fin3));
        fin3.close();

        std::ifstream fin4("tmp_empty_hex.txt", std::ios::binary);
        EXPECT_FALSE(hj::hex::is_valid(fin4));
        fin4.close();

        std::remove("tmp_valid_hex.txt");
        std::remove("tmp_invalid_hex.txt");
        std::remove("tmp_odd_hex.txt");
        std::remove("tmp_empty_hex.txt");
    }
}

TEST(hex, file_encode_decode)
{
    std::string raw_data =
        "High-Jump C++17 Hex Benchmark & Correctness Test Data Stream.";
    std::string in_file  = "tmp_raw_data.bin";
    std::string hex_file = "tmp_encoded.hex";
    std::string out_file = "tmp_decoded.bin";

    {
        std::ofstream ofs(in_file, std::ios::binary);
        ofs << raw_data;
    }

    EXPECT_TRUE(hj::hex::encode_file(hex_file, in_file, true));
    EXPECT_TRUE(hj::hex::is_valid_file(hex_file));

    EXPECT_TRUE(hj::hex::decode_file(out_file, hex_file));

    std::ifstream fin(out_file, std::ios::binary);
    std::string   restored_data((std::istreambuf_iterator<char>(fin)),
                                std::istreambuf_iterator<char>());
    ASSERT_EQ(raw_data, restored_data);

    fin.close();
    std::remove(in_file.c_str());
    std::remove(hex_file.c_str());
    std::remove(out_file.c_str());
}

TEST(hex, stream_operators)
{
    std::ostringstream oss;
    oss << hj::hex::encoder("Hello", true);
    ASSERT_EQ(oss.str(), "48656C6C6F");

    std::ostringstream oss_lower;
    oss_lower << hj::hex::encoder("Hello", false);
    ASSERT_EQ(oss_lower.str(), "48656c6c6f");

    std::vector<uint8_t> bytes = {0x01, 0x02, 0xFF};
    std::ostringstream   oss_vec;
    oss_vec << hj::hex::encoder(bytes, true);
    ASSERT_EQ(oss_vec.str(), "0102FF");

    std::istringstream iss("48656C6C6F");
    std::string        decoded_str;
    iss >> hj::hex::decoder(decoded_str);
    ASSERT_EQ(decoded_str, "Hello");
    EXPECT_FALSE(iss.fail());

    std::istringstream iss_bad("48656C6C6G");
    std::string        bad_str;
    iss_bad >> hj::hex::decoder(bad_str);
    EXPECT_TRUE(iss_bad.fail());
}

TEST(hex, try_decode_optional)
{
    auto opt_zero = hj::hex::try_decode<uint8_t>("00");
    ASSERT_TRUE(opt_zero.has_value());
    ASSERT_EQ(*opt_zero, 0);

    auto opt_failed = hj::hex::try_decode<uint8_t>("GG");
    ASSERT_FALSE(opt_failed.has_value());

    auto opt_overflow = hj::hex::try_decode<uint8_t>("0100");
    ASSERT_FALSE(opt_overflow.has_value());

    EXPECT_THROW(hj::hex::decode<uint8_t>("GG"), std::invalid_argument);
    EXPECT_THROW(hj::hex::decode<uint8_t>("0100"), std::invalid_argument);
}

TEST(hex, stream_operator_generics)
{
    std::istringstream iss1("000000FF");
    uint32_t           val1 = 0;
    iss1 >> hj::hex::decoder(val1);
    ASSERT_EQ(val1, 255);
    EXPECT_FALSE(iss1.fail());

    std::istringstream iss2("00");
    uint8_t            val2 = 0xFF;
    iss2 >> hj::hex::decoder(val2);
    ASSERT_EQ(val2, 0);
    EXPECT_FALSE(iss2.fail());

    std::istringstream iss_bad("GG");
    uint32_t           val_bad = 0;
    iss_bad >> hj::hex::decoder(val_bad);
    EXPECT_TRUE(iss_bad.fail());
}

TEST(hex, overflow_safety)
{
    constexpr std::size_t invalid_len =
        std::numeric_limits<std::size_t>::max() / 2 + 1;
    char dummy_out[10];
    char dummy_in[10] = {0};

    EXPECT_FALSE(hj::hex::try_encode(dummy_out,
                                     sizeof(dummy_out),
                                     static_cast<const void *>(dummy_in),
                                     invalid_len));

    EXPECT_THROW(
        hj::hex::encode(static_cast<const void *>(dummy_in), invalid_len, true),
        std::overflow_error);
}

TEST(hex, file_atomic_safety)
{
    std::string bad_hex_path = "tmp_bad.hex";
    std::string target_path  = "important_data.bin";

    {
        std::ofstream ofs(target_path, std::ios::binary);
        ofs << "ORIGINAL_SAFE_DATA";
    }

    {
        std::ofstream ofs(bad_hex_path, std::ios::binary);
        ofs << "0011GG";
    }

    EXPECT_FALSE(hj::hex::decode_file(target_path, bad_hex_path));

    std::ifstream fin(target_path, std::ios::binary);
    std::string   content((std::istreambuf_iterator<char>(fin)),
                          std::istreambuf_iterator<char>());
    ASSERT_EQ(content, "ORIGINAL_SAFE_DATA");

    fin.close();
    std::remove(bad_hex_path.c_str());
    std::remove(target_path.c_str());
}

TEST(hex, stream_error_propagation)
{
    std::istringstream in("48656C6C6F");
    std::ostringstream out;

    out.setstate(std::ios::failbit);

    EXPECT_FALSE(hj::hex::decode(out, in));
    EXPECT_FALSE(hj::hex::encode(out, in));
}

TEST(hex, stream_badbit_failure)
{
    std::istringstream in("48656C6C6F");
    std::ostringstream out;

    in.setstate(std::ios::badbit);
    EXPECT_FALSE(hj::hex::decode(out, in));
    EXPECT_FALSE(hj::hex::encode(out, in));

    in.clear();
    out.setstate(std::ios::badbit);
    EXPECT_FALSE(hj::hex::decode(out, in));
    EXPECT_FALSE(hj::hex::encode(out, in));
}

TEST(hex, full_byte_space_roundtrip)
{
    std::vector<uint8_t> all_bytes(256);
    for(int i = 0; i < 256; ++i)
    {
        all_bytes[static_cast<std::size_t>(i)] = static_cast<uint8_t>(i);
    }

    std::string encoded_upper = hj::hex::encode(all_bytes, true);
    ASSERT_EQ(encoded_upper.length(), 512);
    auto decoded_upper = hj::hex::decode<std::vector<uint8_t>>(encoded_upper);
    ASSERT_EQ(all_bytes, decoded_upper);

    std::string encoded_lower = hj::hex::encode(all_bytes, false);
    ASSERT_EQ(encoded_lower.length(), 512);
    auto decoded_lower = hj::hex::decode<std::vector<uint8_t>>(encoded_lower);
    ASSERT_EQ(all_bytes, decoded_lower);
}

TEST(hex, large_random_data_roundtrip)
{
    const std::vector<std::size_t> test_sizes = {
        1 * 1024,        // 1 KB
        64 * 1024,       // 64 KB
        1 * 1024 * 1024, // 1 MB
        10 * 1024 * 1024 // 10 MB
    };

    std::mt19937                       rng(42);
    std::uniform_int_distribution<int> dist(0, 255);

    for(std::size_t sz : test_sizes)
    {
        std::vector<uint8_t> original(sz);
        for(std::size_t i = 0; i < sz; ++i)
        {
            original[i] = static_cast<uint8_t>(dist(rng));
        }

        std::string encoded = hj::hex::encode(original, true);
        ASSERT_EQ(encoded.length(), sz * 2);
        auto decoded = hj::hex::decode<std::vector<uint8_t>>(encoded);
        ASSERT_EQ(original, decoded);

        std::string raw_str(reinterpret_cast<const char *>(original.data()),
                            sz);
        std::istringstream raw_in(raw_str, std::ios::binary);
        std::ostringstream hex_out;
        ASSERT_TRUE(hj::hex::encode(hex_out, raw_in, true));

        std::istringstream hex_in(hex_out.str(), std::ios::binary);
        std::ostringstream raw_out;
        ASSERT_TRUE(hj::hex::decode(raw_out, hex_in));

        std::string restored_str = raw_out.str();
        ASSERT_EQ(restored_str.size(), sz);
        ASSERT_EQ(0, std::memcmp(original.data(), restored_str.data(), sz));
    }
}

class CustomChunkBuf : public std::stringbuf
{
  public:
    explicit CustomChunkBuf(const std::string &s, std::size_t chunk_size)
        : std::stringbuf(s, std::ios_base::in | std::ios_base::binary)
        , chunk_size_(chunk_size)
    {
    }

  protected:
    std::streamsize xsgetn(char *s, std::streamsize count) override
    {
        std::streamsize limited_count =
            std::min(count, static_cast<std::streamsize>(chunk_size_));
        return std::stringbuf::xsgetn(s, limited_count);
    }

  private:
    std::size_t chunk_size_;
};

TEST(hex, stream_chunk_boundary)
{
    std::vector<uint8_t> original(8192);
    for(std::size_t i = 0; i < original.size(); ++i)
    {
        original[i] = static_cast<uint8_t>((i * 31 + 17) & 0xFF);
    }
    std::string hex_str = hj::hex::encode(original, true);

    const std::vector<std::size_t> chunk_sizes =
        {1, 2, 3, 4, 7, 15, 16, 4095, 4096, 4097, 8191, 8192, 8193};

    for(std::size_t chunk_sz : chunk_sizes)
    {
        CustomChunkBuf     custom_buf(hex_str, chunk_sz);
        std::istream       in(&custom_buf);
        std::ostringstream out;

        ASSERT_TRUE(hj::hex::decode(out, in))
            << "Failed at chunk_size = " << chunk_sz;

        std::string decoded_str = out.str();
        ASSERT_EQ(decoded_str.size(), original.size())
            << "Size mismatch at chunk_size = " << chunk_sz;
        ASSERT_EQ(
            0,
            std::memcmp(original.data(), decoded_str.data(), original.size()))
            << "Data corruption at chunk_size = " << chunk_sz;
    }
}

TEST(hex, stream_encode_chunk_boundary)
{
    std::string raw_data(8192, '\0');
    for(std::size_t i = 0; i < raw_data.size(); ++i)
    {
        raw_data[i] = static_cast<char>((i * 37 + 13) & 0xFF);
    }
    std::string expected_hex = hj::hex::encode(raw_data, true);

    const std::vector<std::size_t> chunk_sizes =
        {1, 2, 3, 4, 7, 15, 16, 4095, 4096, 4097, 8191, 8192, 8193};

    for(std::size_t chunk_sz : chunk_sizes)
    {
        CustomChunkBuf     custom_buf(raw_data, chunk_sz);
        std::istream       in(&custom_buf);
        std::ostringstream out;

        ASSERT_TRUE(hj::hex::encode(out, in, true))
            << "Encode failed at chunk_size = " << chunk_sz;

        ASSERT_EQ(out.str(), expected_hex)
            << "Encoded data mismatch at chunk_size = " << chunk_sz;
    }
}

TEST(hex, malformed_inputs_fuzz_simulation)
{
    const std::vector<std::string> malformed_cases = {"",
                                                      "0",
                                                      "000",
                                                      "GG",
                                                      "0G",
                                                      "G0",
                                                      "GGGG",
                                                      "ABCDEFZ",
                                                      " 00",
                                                      "00 ",
                                                      "00\n",
                                                      "00\t",
                                                      "0x12",
                                                      "1234 ",
                                                      "FF FF"};

    for(const auto &bad_input : malformed_cases)
    {
        EXPECT_FALSE(hj::hex::is_valid(bad_input))
            << "is_valid failed to reject: [" << bad_input << "]";

        EXPECT_THROW(hj::hex::decode<std::string>(bad_input),
                     std::invalid_argument)
            << "decode failed to throw on: [" << bad_input << "]";

        EXPECT_FALSE(hj::hex::try_decode<std::string>(bad_input).has_value())
            << "try_decode failed to return nullopt on: [" << bad_input << "]";

        std::istringstream in(bad_input);
        std::ostringstream out;
        EXPECT_FALSE(hj::hex::decode(out, in))
            << "Stream decode failed to return false on: [" << bad_input << "]";
    }

    std::mt19937                       rng(1337);
    std::uniform_int_distribution<int> char_dist(0, 255);

    for(int i = 0; i < 5000; ++i)
    {
        std::string fuzz_str;
        std::size_t len = static_cast<std::size_t>(i % 32);
        fuzz_str.reserve(len);
        for(std::size_t j = 0; j < len; ++j)
        {
            fuzz_str.push_back(static_cast<char>(char_dist(rng)));
        }

        bool is_valid_hex = hj::hex::is_valid(fuzz_str);
        if(!is_valid_hex)
        {
            EXPECT_THROW(hj::hex::decode<std::string>(fuzz_str),
                         std::invalid_argument);
            EXPECT_FALSE(
                hj::hex::try_decode<std::string>(fuzz_str).has_value());
        } else
        {
            EXPECT_NO_THROW(hj::hex::decode<std::string>(fuzz_str));
            EXPECT_TRUE(hj::hex::try_decode<std::string>(fuzz_str).has_value());
        }
    }
}

TEST(hex, integral_boundaries_and_overflows)
{
    EXPECT_EQ(hj::hex::decode<uint8_t>("00"), 0x00);
    EXPECT_EQ(hj::hex::decode<uint8_t>("FF"), 0xFF);

    EXPECT_EQ(hj::hex::decode<uint16_t>("0000"), 0x0000);
    EXPECT_EQ(hj::hex::decode<uint16_t>("FFFF"), 0xFFFF);

    EXPECT_EQ(hj::hex::decode<uint32_t>("00000000"), 0x00000000);
    EXPECT_EQ(hj::hex::decode<uint32_t>("FFFFFFFF"), 0xFFFFFFFF);

    EXPECT_EQ(hj::hex::decode<uint64_t>("0000000000000000"),
              0x0000000000000000ULL);
    EXPECT_EQ(hj::hex::decode<uint64_t>("FFFFFFFFFFFFFFFF"),
              0xFFFFFFFFFFFFFFFFULL);

    EXPECT_FALSE(hj::hex::try_decode<uint8_t>("100").has_value());
    EXPECT_FALSE(hj::hex::try_decode<uint8_t>("0100").has_value());
    EXPECT_THROW(hj::hex::decode<uint8_t>("0100"), std::invalid_argument);

    EXPECT_FALSE(hj::hex::try_decode<uint16_t>("10000").has_value());
    EXPECT_FALSE(hj::hex::try_decode<uint16_t>("010000").has_value());
    EXPECT_THROW(hj::hex::decode<uint16_t>("10000"), std::invalid_argument);

    EXPECT_FALSE(hj::hex::try_decode<uint32_t>("100000000").has_value());
    EXPECT_FALSE(hj::hex::try_decode<uint32_t>("1FFFFFFFF").has_value());

    EXPECT_FALSE(
        hj::hex::try_decode<uint64_t>("10000000000000000").has_value());
    EXPECT_FALSE(
        hj::hex::try_decode<uint64_t>("1FFFFFFFFFFFFFFFF").has_value());
}