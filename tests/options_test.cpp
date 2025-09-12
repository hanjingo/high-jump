#include <gtest/gtest.h>
#include <libcpp/os/options.hpp>
#include <vector>
#include <string>
#include <cstring>

#include <algorithm>

using namespace libcpp;

class OptionsTest : public ::testing::Test {
protected:
    void SetUp() override {
        opts = std::make_unique<options>();
    }

    void TearDown() override {
        opts.reset();
        for (auto ptr : allocated_args) {
            delete[] ptr;
        }
        // allocated_args.clear();
        for (auto arr : allocated_argv_arrays) {
            delete[] arr;
        }
        allocated_argv_arrays.clear();
    }

    char** create_argv(const std::vector<std::string>& args) {
        char** argv = new char*[args.size()];
        // allocated_args.clear();
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i] = new char[args[i].length() + 1];
            std::strcpy(argv[i], args[i].c_str());
            allocated_args.push_back(argv[i]);
        }
        allocated_argv_arrays.push_back(argv); // 跟踪 argv 指针数组
        return argv;
    }

    std::unique_ptr<options> opts;
    std::vector<char*> allocated_args;
    std::vector<char**> allocated_argv_arrays; // 新增
};

TEST_F(OptionsTest, ParseIntegerOption) {
    opts->add<int>("port", 8080, "Server port number");
    
    std::vector<std::string> args = {"program", "--port", "9000"};
    char** argv = create_argv(args);
    
    int result = opts->parse<int>(static_cast<int>(args.size()), argv, "port");
    EXPECT_EQ(result, 9000);
}

TEST_F(OptionsTest, ParseIntegerOptionWithDefault) {
    opts->add<int>("port", 8080, "Server port number");
    
    std::vector<std::string> args = {"program"};
    char** argv = create_argv(args);
    
    int result = opts->parse<int>(static_cast<int>(args.size()), argv, "port");
    EXPECT_EQ(result, 8080);
}

TEST_F(OptionsTest, ParseStringOption) {
    opts->add<std::string>("host", "localhost", "Server hostname");
    
    std::vector<std::string> args = {"program", "--host", "example.com"};
    char** argv = create_argv(args);
    
    std::string result = opts->parse<std::string>(static_cast<int>(args.size()), argv, "host");
    EXPECT_EQ(result, "example.com");
}

TEST_F(OptionsTest, ParseStringOptionWithDefault) {
    opts->add<std::string>("host", "localhost", "Server hostname");
    
    std::vector<std::string> args = {"program"};
    char** argv = create_argv(args);
    
    std::string result = opts->parse<std::string>(static_cast<int>(args.size()), argv, "host");
    EXPECT_EQ(result, "localhost");
}

TEST_F(OptionsTest, ParseFloatOption) {
    opts->add<float>("timeout", 30.5f, "Connection timeout in seconds");
    
    std::vector<std::string> args = {"program", "--timeout", "45.25"};
    char** argv = create_argv(args);
    
    float result = opts->parse<float>(static_cast<int>(args.size()), argv, "timeout");
    EXPECT_FLOAT_EQ(result, 45.25f);
}

TEST_F(OptionsTest, ParseDoubleOption) {
    opts->add<double>("precision", 0.001, "Calculation precision");
    
    std::vector<std::string> args = {"program", "--precision", "0.0001"};
    char** argv = create_argv(args);
    
    double result = opts->parse<double>(static_cast<int>(args.size()), argv, "precision");
    EXPECT_DOUBLE_EQ(result, 0.0001);
}

TEST_F(OptionsTest, ParseBoolOption) {
    opts->add<bool>("verbose", false, "Enable verbose output");
    
    std::vector<std::string> args = {"program", "--verbose", "true"};
    char** argv = create_argv(args);
    
    bool result = opts->parse<bool>(static_cast<int>(args.size()), argv, "verbose");
    EXPECT_TRUE(result);
}

TEST_F(OptionsTest, ParseBoolOptionVariations) {
    opts->add<bool>("debug", true, "Enable debug mode");
    
    std::vector<std::string> args = {"program", "--debug", "false"};
    char** argv = create_argv(args);
    
    bool result = opts->parse<bool>(static_cast<int>(args.size()), argv, "debug");
    EXPECT_FALSE(result);

    std::vector<std::string> args2 = {"program", "--debug", "1"};
    char** argv2 = create_argv(args2);
    
    bool result2 = opts->parse<bool>(static_cast<int>(args2.size()), argv2, "debug");
    EXPECT_TRUE(result2);
}

TEST_F(OptionsTest, ParseMultipleOptions) {
    opts->add<std::string>("host", "localhost", "Server hostname");
    opts->add<int>("port", 8080, "Server port number");
    opts->add<bool>("verbose", false, "Enable verbose output");
    
    std::vector<std::string> args = {
        "program", "--host", "example.com", "--port", "9000", "--verbose", "true"
    };
    char** argv = create_argv(args);
    
    std::string host = opts->parse<std::string>(static_cast<int>(args.size()), argv, "host");
    int port = opts->parse<int>(static_cast<int>(args.size()), argv, "port");
    bool verbose = opts->parse<bool>(static_cast<int>(args.size()), argv, "verbose");
    
    EXPECT_EQ(host, "example.com");
    EXPECT_EQ(port, 9000);
    EXPECT_TRUE(verbose);
}

TEST_F(OptionsTest, ParseNonExistentKey) {
    opts->add<int>("port", 8080, "Server port number");
    
    std::vector<std::string> args = {"program", "--port", "9000"};
    char** argv = create_argv(args);
    int result = opts->parse<int>(static_cast<int>(args.size()), argv, "timeout", -1);
    EXPECT_EQ(result, -1);

    int result2 = opts->parse<int>(static_cast<int>(args.size()), argv, "timeout");
    EXPECT_EQ(result2, 0);
}

TEST_F(OptionsTest, ParseInvalidValue) {
    opts->add<int>("port", 8080, "Server port number");
    
    std::vector<std::string> args = {"program", "--port", "invalid_number"};
    char** argv = create_argv(args);

    int result = opts->parse<int>(static_cast<int>(args.size()), argv, "port", 9999);
    EXPECT_EQ(result, 9999);
}

TEST_F(OptionsTest, ParseSuccessIgnoresDefault) {
    opts->add<int>("port", 8080, "Server port number");
    
    std::vector<std::string> args = {"program", "--port", "9000"};
    char** argv = create_argv(args);
    
    int result = opts->parse<int>(static_cast<int>(args.size()), argv, "port", 1234);
    EXPECT_EQ(result, 9000);
}

TEST_F(OptionsTest, ParseWithDifferentTypeDefaults) {
    opts->add<std::string>("host", "localhost", "Server hostname");
    opts->add<bool>("verbose", false, "Enable verbose output");
    opts->add<double>("ratio", 0.5, "Processing ratio");
    
    std::vector<std::string> args = {"program"};
    char** argv = create_argv(args);

    std::string host = opts->parse<std::string>(static_cast<int>(args.size()), argv, "host", "default.com");
    EXPECT_EQ(host, "default.com");

    bool verbose = opts->parse<bool>(static_cast<int>(args.size()), argv, "verbose", true);
    EXPECT_TRUE(verbose);

    double ratio = opts->parse<double>(static_cast<int>(args.size()), argv, "ratio", 0.75);
    EXPECT_DOUBLE_EQ(ratio, 0.75);
}

TEST_F(OptionsTest, ExplicitValueIgnoresParseDefault) {
    opts->add<std::string>("host", "localhost", "Server hostname");
    opts->add<bool>("verbose", false, "Enable verbose output");
    opts->add<double>("ratio", 0.5, "Processing ratio");
    
    std::vector<std::string> args = {
        "program", 
        "--host", "example.com",
        "--verbose", "false",
        "--ratio", "0.25"
    };
    char** argv = create_argv(args);

    std::string host = opts->parse<std::string>(static_cast<int>(args.size()), argv, "host", "default.com");
    EXPECT_EQ(host, "example.com");
    
    bool verbose = opts->parse<bool>(static_cast<int>(args.size()), argv, "verbose", true);
    EXPECT_FALSE(verbose);
    
    double ratio = opts->parse<double>(static_cast<int>(args.size()), argv, "ratio", 0.75);
    EXPECT_DOUBLE_EQ(ratio, 0.25);
}

TEST_F(OptionsTest, AddDefaultVsParseDefault) {
    opts->add<int>("port", 8080, "Server port");
    opts->add<std::string>("host", "localhost", "Server host");

    std::vector<std::string> args1 = {"program"};
    char** argv1 = create_argv(args1);
    
    int port1 = opts->parse<int>(static_cast<int>(args1.size()), argv1, "port");
    std::string host1 = opts->parse<std::string>(static_cast<int>(args1.size()), argv1, "host");
    
    EXPECT_EQ(port1, 8080);
    EXPECT_EQ(host1, "localhost");

    std::vector<std::string> args2 = {"program"};
    char** argv2 = create_argv(args2);
    
    int port2 = opts->parse<int>(static_cast<int>(args2.size()), argv2, "port", 9000);
    std::string host2 = opts->parse<std::string>(static_cast<int>(args2.size()), argv2, "host", "example.com");
    
    EXPECT_EQ(port2, 9000);
    EXPECT_EQ(host2, "example.com");
}

TEST_F(OptionsTest, ParseComplexScenarioWithDefaults) {
    opts->add<int>("workers", 4, "Number of worker threads");
    opts->add<std::string>("database", "local.db", "Database connection string");
    opts->add<bool>("debug", false, "Enable debug mode");
    
    std::vector<std::string> args = {
        "program", 
        "--workers", "8",
        "--database", "prod.db",
    };
    char** argv = create_argv(args);

    int workers = opts->parse<int>(static_cast<int>(args.size()), argv, "workers", 1);
    EXPECT_EQ(workers, 8);
    
    std::string db = opts->parse<std::string>(static_cast<int>(args.size()), argv, "database", "memory.db");
    EXPECT_EQ(db, "prod.db");

    bool debug = opts->parse<bool>(static_cast<int>(args.size()), argv, "debug", true);
    EXPECT_TRUE(debug);

    int timeout = opts->parse<int>(static_cast<int>(args.size()), argv, "timeout", 30);
    EXPECT_EQ(timeout, 30);
}

TEST_F(OptionsTest, BackwardCompatibilityAndEdgeCases) {
    opts->add<int>("value", 42, "Test value");
    
    std::vector<std::string> args = {"program", "--value", "100"};
    char** argv = create_argv(args);

    int old_style = opts->parse<int>(static_cast<int>(args.size()), argv, "value");
    EXPECT_EQ(old_style, 100);

    int new_style = opts->parse<int>(static_cast<int>(args.size()), argv, "value", 999);
    EXPECT_EQ(new_style, 100);

    int old_missing = opts->parse<int>(static_cast<int>(args.size()), argv, "missing");
    EXPECT_EQ(old_missing, 0);
    
    int new_missing = opts->parse<int>(static_cast<int>(args.size()), argv, "missing", 777);
    EXPECT_EQ(new_missing, 777);
}

TEST_F(OptionsTest, ParseBoundaryValues) {
    opts->add<int>("int_val", 0, "Integer value");
    opts->add<float>("float_val", 0.0f, "Float value");

    std::vector<std::string> args1 = {"program", "--int_val", "2147483647"};  // INT_MAX
    char** argv1 = create_argv(args1);
    
    int int_result = opts->parse<int>(static_cast<int>(args1.size()), argv1, "int_val");
    EXPECT_EQ(int_result, 2147483647);


    std::vector<std::string> args2 = {"program", "--int_val", "-2147483648"};  // INT_MIN
    char** argv2 = create_argv(args2);
    
    int int_result2 = opts->parse<int>(static_cast<int>(args2.size()), argv2, "int_val");
    EXPECT_EQ(int_result2, -2147483648);
}

TEST_F(OptionsTest, TypeSafety) {
    opts->add<int>("number", 42, "A number");
    
    std::vector<std::string> args = {"program", "--number", "123"};
    char** argv = create_argv(args);

    std::string wrong_type = opts->parse<std::string>(static_cast<int>(args.size()), argv, "number");
    EXPECT_EQ(wrong_type, "");

    int correct_type = opts->parse<int>(static_cast<int>(args.size()), argv, "number");
    EXPECT_EQ(correct_type, 123);
}

TEST_F(OptionsTest, ParseSinglePositionalArgument) {
    opts->add<std::string>("input", "", "input file");
    opts->add_positional("input", 1);
    std::vector<std::string> args = {"program", "file.txt"};
    char** argv = create_argv(args);
    std::string input = opts->parse<std::string>(static_cast<int>(args.size()), argv, "input");
    EXPECT_EQ(input, "file.txt");
}

TEST_F(OptionsTest, ParseMultiplePositionalArguments) {
    opts->add<std::string>("input", "", "input file");
    opts->add<std::string>("output", "", "output file");
    opts->add_positional("input", 1);
    opts->add_positional("output", 1);
    std::vector<std::string> args = {"program", "in.txt", "out.txt"};
    char** argv = create_argv(args);
    std::string input = opts->parse<std::string>(static_cast<int>(args.size()), argv, "input");
    std::string output = opts->parse<std::string>(static_cast<int>(args.size()), argv, "output");
    EXPECT_EQ(input, "in.txt");
    EXPECT_EQ(output, "out.txt");
}

TEST_F(OptionsTest, ParsePositionalVector) {
    opts->add<std::vector<std::string>>("files", {}, "input files");
    opts->add_positional("files", 3);
    std::vector<std::string> args = {"program", "a.txt", "b.txt", "c.txt"};
    char** argv = create_argv(args);
    auto files = opts->parse_positional<std::vector<std::string>>(static_cast<int>(args.size()), argv, "files");
    ASSERT_EQ(files.size(), 3);
    EXPECT_TRUE(std::find(files.begin(), files.end(), "a.txt") != files.end());
    EXPECT_TRUE(std::find(files.begin(), files.end(), "b.txt") != files.end());
    EXPECT_TRUE(std::find(files.begin(), files.end(), "c.txt") != files.end());
}

TEST_F(OptionsTest, ParseMixedOptionsAndPositional) {
    opts->add<std::string>("output,o", "", "output file");
    opts->add<std::string>("input", "", "input file");
    opts->add_positional("input", 1);
    std::vector<std::string> args = {"program", "input.txt", "-o", "output.txt"};
    char** argv = create_argv(args);
    std::string input = opts->parse<std::string>(static_cast<int>(args.size()), argv, "input");
    std::string output = opts->parse<std::string>(static_cast<int>(args.size()), argv, "output");
    EXPECT_EQ(input, "input.txt");
    EXPECT_EQ(output, "output.txt");
}