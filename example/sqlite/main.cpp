#include <iostream>
#include <thread>
#include <chrono>

#include <libcpp/db/sqlite.hpp>

int select_cb(void* in, int argc, char** argv, char** col_name)
{
    std::cout << ">>On Select Row CallBack" << std::endl;
    if (in != NULL) {
        std::cout << "in = " << static_cast<std::string*>(in)->c_str() << std::endl;
    }

    int i;
    for (i = 0; i < argc; i++) {
        std::string key = col_name[i];
        std::string value = argv[i] ? argv[i] : "NULL";
        std::cout << key << ":" << value << std::endl;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    char* err = 0;
    sqlite3* db;
    char** result;
    int row;
    int col;

    // open
    int ret = sqlite3_open("007.db", &db);
    if (ret != SQLITE_OK) {
        std::cout << "open db fail with err = " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }
    std::cout << "open db success\n" << std::endl;

    // create table
    std::string sql = "CREATE TABLE libcpp(" \
                      "id int primary key NOT NULL," \
                      "name TEXT NOT NULL);";
    ret = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
    if (ret != SQLITE_OK) {
        std::cout << "create table fail with err = " << err << std::endl;
        return 0;
    }
    std::cout << "create db table success\n" << std::endl;

    // insert row
    sql = "INSERT INTO libcpp (id, name) VALUES (1, 'he');";
    ret = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
    if (ret != SQLITE_OK) {
        std::cout << "insert table fail with err = " << err << std::endl;
        return 0;
    }
    std::cout << "insert db table success\n" << std::endl;

    // async select row
    std::string hello = "hello";
    sql = "SELECT * FROM libcpp";
    ret = sqlite3_exec(db, sql.c_str(), select_cb, static_cast<void*>(&hello), &err);
    if (ret != SQLITE_OK) {
        std::cout << "async select table fail with err = " << err << std::endl;
        return 0;
    }
    std::cout << "async select db table success\n" << std::endl;

    // sync select row
    ret = sqlite3_get_table(db, sql.c_str(), &result, &row, &col, &err);
    if (ret != SQLITE_OK) {
        std::cout << "sync select table fail with err = " << err << std::endl;
        return 0;
    }
    std::cout << "sync select db table success with:" << std::endl;
    int idx = col;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            std::cout << result[j] << ":" << result[idx] << std::endl;
            idx++;
        }
    }
    std::cout << "\n" << std::endl;

    // update row
    sql = "UPDATE libcpp SET name = 'libcpp' WHERE id = 1";
    ret = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
    if (ret != SQLITE_OK) {
        std::cout << "update table fail with err = " << err << std::endl;
        return 0;
    }
    sql = "SELECT * FROM libcpp";
    ret = sqlite3_exec(db, sql.c_str(), select_cb, 0, &err);
    if (ret != SQLITE_OK) {
        std::cout << "update table fail with err = " << err << std::endl;
        return 0;
    }
    std::cout << "update table success\n" << std::endl;

    // delete row
    std::cout << "start delete row" << std::endl;
    sql = "DELETE FROM libcpp WHERE id = 1";
    ret = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
    if (ret != SQLITE_OK) {
        std::cout << "delete row fail with err = " << err << std::endl;
        return 0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sql = "SELECT * FROM libcpp";
    ret = sqlite3_exec(db, sql.c_str(), select_cb, 0, &err);
    if (ret != SQLITE_OK) {
        std::cout << "delete table fail with err = " << err << std::endl;
        return 0;
    }
    std::cout << "delete row success\n" << std::endl;

    // close db
    sqlite3_close(db);

    std::cin.get();
    return 0;
}