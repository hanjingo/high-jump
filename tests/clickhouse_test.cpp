// NOTE: please install & run clickhouse database before run this testing file
#include <gtest/gtest.h>
#include <thread>

#if CLICKHOUSE_ENABLE

// #ifndef _WIN32
#include <hj/db/clickhouse.hpp>

std::time_t now()
{
    return std::time(nullptr);
}

TEST(clickhouse_client, connect)
{
    hj::ck::client cli("localhost", 9000, "default", "livermore", "default");
    ASSERT_TRUE(cli.is_connected());
}

TEST(clickhouse_client, create_table)
{
    hj::ck::client cli("localhost", 9000, "default", "livermore", "default");
    cli.execute("DROP TABLE IF EXISTS create_table_test");
    cli.execute(R"(
        CREATE TABLE create_table_test (
            id          UInt64,
            age         Int8,
            salary      Decimal(6, 2),
            name        String,
            create_time DateTime
        ) ENGINE = Memory
    )");
}

TEST(clickhouse_client, insert)
{
    hj::ck::client cli("localhost", 9000, "default", "livermore", "default");
    cli.execute("DROP TABLE IF EXISTS insert_test");
    cli.execute(R"(
        CREATE TABLE insert_test (
            id          UInt64,
            age         Int8,
            salary      Decimal(6, 2),
            name        String,
            create_time DateTime
        ) ENGINE = Memory
    )");

    hj::ck::block b;
    auto          id          = std::make_shared<hj::ck::column_uint64>();
    auto          age         = std::make_shared<hj::ck::column_int8>();
    auto          salary      = std::make_shared<hj::ck::column_decimal>(6, 2);
    auto          name        = std::make_shared<hj::ck::column_string>();
    auto          create_time = std::make_shared<hj::ck::column_date_time>();
    for(uint32_t i = 1; i <= 5; ++i)
    {
        id->Append(i);
        age->Append(30);
        salary->Append("123.45");
        name->Append("User-" + std::to_string(i));
        create_time->Append(now());
    }

    b.AppendColumn("id", id);
    b.AppendColumn("age", age);
    b.AppendColumn("salary", salary);
    b.AppendColumn("name", name);
    b.AppendColumn("create_time", create_time);

    cli.insert("insert_test", b);
}

TEST(clickhouse_client, select)
{
    hj::ck::client cli("localhost", 9000, "default", "livermore", "default");
    cli.execute("DROP TABLE IF EXISTS select_test");
    cli.execute(R"(
        CREATE TABLE select_test (
            id    UInt32,
            name  String,
            score Float64,
            dt    DateTime
        ) ENGINE = Memory
    )");

    hj::ck::block b;
    auto          id    = std::make_shared<hj::ck::column_uint32>();
    auto          name  = std::make_shared<hj::ck::column_string>();
    auto          score = std::make_shared<hj::ck::column_float64>();
    auto          dt    = std::make_shared<hj::ck::column_date_time>();
    for(uint32_t i = 1; i <= 5; ++i)
    {
        id->Append(i);
        name->Append("User-" + std::to_string(i));
        score->Append(80.0 + i);
        dt->Append(now());
    }

    b.AppendColumn("id", id);
    b.AppendColumn("name", name);
    b.AppendColumn("score", score);
    b.AppendColumn("dt", dt);

    cli.insert("select_test", b);

    auto results =
        cli.select("SELECT id, name, score, dt FROM select_test WHERE id = 1");
    size_t total_rows = 0;
    for(const auto &block : results)
    {
        total_rows += block.GetRowCount();
        if(block.GetRowCount() > 0)
        {
            auto id    = block[0]->As<hj::ck::column_uint32>()->At(0);
            auto name  = block[1]->As<hj::ck::column_string>()->At(0);
            auto score = block[2]->As<hj::ck::column_float64>()->At(0);
            auto dt    = block[3]->As<hj::ck::column_date_time>()->At(0);
            ASSERT_EQ(id, 1);
            ASSERT_EQ(score, 81.0);
        }
    }
    ASSERT_EQ(total_rows, 1);
}

TEST(clickhouse_client, drop_table)
{
    hj::ck::client cli("localhost", 9000, "default", "livermore", "default");
    cli.execute("DROP TABLE IF EXISTS drop_table_test");
    cli.execute(R"(
        CREATE TABLE drop_table_test (
            id    UInt32,
            name  String,
            score Float64,
            dt    DateTime
        ) ENGINE = Memory
    )");
    cli.execute("DROP TABLE IF EXISTS drop_table_test");
}

#endif // CLICKHOUSE_ENABLE