#include <thread>

#include <gtest/gtest.h>

#ifndef _WIN32
#include <libcpp/db/clickhouse.hpp>

std::time_t now()
{
    return std::time(nullptr);
}

// please install & run clickhouse database before run this testing file

TEST(clickhouse, connect)
{
    libcpp::ck::client_options opts;
    opts.SetHost("localhost")
        .SetPort(9000)
        .SetUser("default")
        .SetPassword("livermore")
        .SetDefaultDatabase("default");
    libcpp::ck::client cli{ opts };
    if (!cli.GetCurrentEndpoint())
        return;
    ASSERT_EQ(cli.GetCurrentEndpoint().has_value(), true);
}

// TEST(clickhouse, create_table)
// {
//     libcpp::ck::client_options opts;
//     opts.SetHost("localhost")
//         .SetPort(9000)
//         .SetUser("default")
//         .SetPassword("livermore")
//         .SetDefaultDatabase("default");
//     libcpp::ck::client cli{opts};
//     if (!cli.GetCurrentEndpoint())
//         return;

//     cli.Execute("DROP TABLE IF EXISTS create_table_test");
//     cli.Execute(R"(
//         CREATE TABLE create_table_test (
//             id          UInt64,
//             age         Int8,
//             salary      Decimal(6, 2),
//             name        String,
//             create_time DateTime
//         ) ENGINE = Memory
//     )");
// }

// TEST(clickhouse, insert)
// {
//     libcpp::ck::client_options opts;
//     opts.SetHost("localhost")
//         .SetPort(9000)
//         .SetUser("default")
//         .SetPassword("livermore")
//         .SetDefaultDatabase("default");
//     libcpp::ck::client cli{opts};
//     if (!cli.GetCurrentEndpoint())
//         return;

//     cli.Execute("DROP TABLE IF EXISTS insert_test");
//     cli.Execute(R"(
//         CREATE TABLE insert_test (
//             id          UInt64,
//             age         Int8,
//             salary      Decimal(6, 2),
//             name        String,
//             create_time DateTime
//         ) ENGINE = Memory
//     )");

//     libcpp::ck::block b;
//     auto id = std::make_shared<libcpp::ck::column_uint64>();
//     auto age = std::make_shared<libcpp::ck::column_int8>();
//     auto salary = std::make_shared<libcpp::ck::column_decimal>(6, 2);
//     auto name = std::make_shared<libcpp::ck::column_string>();
//     auto create_time = std::make_shared<libcpp::ck::column_date_time>();
//     for (uint32_t i = 1; i <= 5; ++i)
//     {
//         id->Append(i);
//         age->Append(30);
//         salary->Append("123.45");
//         name->Append("User-" + std::to_string(i));
//         create_time->Append(now());
//     }

//     b.AppendColumn("id", id);
//     b.AppendColumn("age", age);
//     b.AppendColumn("salary", salary);
//     b.AppendColumn("name", name);
//     b.AppendColumn("create_time", create_time);

//     cli.Insert("insert_test", b);
// }

// TEST(clickhouse, select)
// {
//     libcpp::ck::client_options opts;
//     opts.SetHost("localhost")
//         .SetPort(9000)
//         .SetUser("default")
//         .SetPassword("livermore")
//         .SetDefaultDatabase("default");
//     libcpp::ck::client cli{opts};
//     if (!cli.GetCurrentEndpoint())
//         return;

//     cli.Execute("DROP TABLE IF EXISTS select_test");
//     cli.Execute(R"(
//         CREATE TABLE select_test (
//             id    UInt32,
//             name  String,
//             score Float64,
//             dt    DateTime
//         ) ENGINE = Memory
//     )");

//     libcpp::ck::block b;
//     auto id = std::make_shared<libcpp::ck::column_uint32>();
//     auto name = std::make_shared<libcpp::ck::column_string>();
//     auto score = std::make_shared<libcpp::ck::column_float64>();
//     auto dt = std::make_shared<libcpp::ck::column_date_time>();
//     for (uint32_t i = 1; i <= 5; ++i)
//     {
//         id->Append(i);
//         name->Append("User-" + std::to_string(i));
//         score->Append(80.0 + i);
//         dt->Append(now());
//     }

//     b.AppendColumn("id", id);
//     b.AppendColumn("name", name);
//     b.AppendColumn("score", score);
//     b.AppendColumn("dt", dt);

//     cli.Insert("select_test", b);

//     cli.Select("SELECT id, name, score, dt FROM select_test WHERE id = 1",
//     [](const libcpp::ck::block& block){
//         for (size_t i = 0; i < block.GetRowCount(); ++i) {
//             auto id    = block[0]->As<libcpp::ck::column_uint32>()->At(i);
//             auto name  = block[1]->As<libcpp::ck::column_string>()->At(i);
//             auto score = block[2]->As<libcpp::ck::column_float64>()->At(i);
//             auto dt    = block[3]->As<libcpp::ck::column_date_time>()->At(i);

//             ASSERT_EQ(id, 1);
//             ASSERT_EQ(score, 81.0 + i);
//         }
//     });
// }

// TEST(clickhouse, drop_table)
// {
//     libcpp::ck::client_options opts;
//     opts.SetHost("localhost")
//         .SetPort(9000)
//         .SetUser("default")
//         .SetPassword("livermore")
//         .SetDefaultDatabase("default");
//     libcpp::ck::client cli{opts};
//     if (!cli.GetCurrentEndpoint())
//         return;

//     cli.Execute("DROP TABLE IF EXISTS drop_table_test");
//     cli.Execute(R"(
//         CREATE TABLE drop_table_test (
//             id    UInt32,
//             name  String,
//             score Float64,
//             dt    DateTime
//         ) ENGINE = Memory
//     )");
//     cli.Execute("DROP TABLE IF EXISTS drop_table_test");
// }

#endif