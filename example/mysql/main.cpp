#include <iostream>
#include <string>
#include <string.h>

#include <libcpp/db/mysql.hpp>

int main(int argc, char* argv[])
{
    char sql[200];

    MYSQL mysql;
    MYSQL_RES* ret;
    MYSQL_ROW row;

    mysql_init(&mysql);

    mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");

    std::string ip = "localhost";
    std::string name = "root";
    std::string passwd = "";
    std::string db = "he";
    if (mysql_real_connect(&mysql, ip.c_str(), name.c_str(), passwd.c_str(),
                           db.c_str(), 3306, NULL, 0) == NULL) {
        std::cout << "connect mysql fail" << std::endl;
        return 0;
    }

    memset(sql, 0, 200);
    sprintf(sql, "insert into t1 values('%s', %d)", "he", 30);
    mysql_query(&mysql, sql);

    memset(sql, 0, 200);
    sprintf(sql, "update t1 set age = %d wher name = '%s'", 31, "he");
    mysql_query(&mysql, sql);

    mysql_query(&mysql, "select * from t1");
    ret = mysql_store_result(&mysql);
    while (row = mysql_fetch_row(ret)) {
        std::cout << row[0] << ", " << row[1] << std::endl;
    }

    memset(sql, 0, 200);
    sprintf(sql, "delete from t1 wher name = '%s'", "he");
    mysql_query(&mysql, sql);

    mysql_free_result(ret);
    mysql_close(&mysql);

    std::cin.get();
    return 0;
}