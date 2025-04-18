#ifndef CLICKHOUSE_HPP
#define CLICKHOUSE_HPP

#include <sql.h>
#include <sqlext.h>

// SQLHENV hEnv;
//     SQLHDBC hDbc;
//     SQLRETURN ret;
//     SQLCHAR outConnStr[1024];
 
//     // 分配环境句柄
//     SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
//     SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
 
//     // 分配连接句柄
//     SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
 
//     // 连接数据库
//     ret = SQLConnect(hDbc, (SQLCHAR*)"DSN=mydsn;UID=user;PWD=password", SQL_NTS);
//     if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
//         std::cout << "Connected successfully!" << std::endl;
//     } else {
//         std::cout << "Connection failed!" << std::endl;
//         SQLError(hEnv, hDbc, 0, outConnStr, 0, 0, 0);
//         std::cout << "Error: " << outConnStr << std::endl;
//     }
 
//     // 释放句柄并退出
//     SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
//     SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

namespace libcpp
{

// TODO

}

#endif