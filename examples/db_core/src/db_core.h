#ifndef DB_CORE_API_H
#define DB_CORE_API_H

#include <hj/os/dll.h>

// ---------------- error code --------------------
#ifndef DB_ERR_START
#define DB_ERR_START 0
#endif

#ifndef OK
#define OK                                     (DB_ERR_START)
#endif

#define DB_ERR_FAIL                            (DB_ERR_START + 1)
#define DB_ERR_CORE_NOT_LOAD                   (DB_ERR_START + 2)
#define DB_ERR_CORE_LOAD_FAIL                  (DB_ERR_START + 3)
#define DB_ERR_VERSION_MISMATCH                (DB_ERR_START + 4)
#define DB_ERR_INIT_FAIL                       (DB_ERR_START + 5)
#define DB_ERR_QUIT_FAIL                       (DB_ERR_START + 6)
#define DB_ERR_DB_EXISTED                      (DB_ERR_START + 7)
#define DB_ERR_DB_NOT_EXIST                    (DB_ERR_START + 8)
#define DB_ERR_REQUIRE_FAIL                    (DB_ERR_START + 9)
#define DB_ERR_PARSE_INIT_OPTION_FAIL          (DB_ERR_START + 10)

#define DB_ERR_SQLITE_EXEC_FAIL                (DB_ERR_START + 101)
#define DB_ERR_SQLITE_NOT_OPEN                 (DB_ERR_START + 102)
#define DB_ERR_SQLITE_ALREADY_OPEN             (DB_ERR_START + 103)
#define DB_ERR_SQLITE_OPEN_FAIL                (DB_ERR_START + 104)
#define DB_ERR_SQLITE_GET_CONN_FAIL            (DB_ERR_START + 105)

#ifndef DB_ERR_END
#define DB_ERR_END                             (DB_ERR_START + 999)
#endif

// --------------- const --------------------
#define DB_MAX_DATA_POOL_SIZE 1024

#define DB_MAX_OUTPUT_NUM 1024 // TOTAL: 16KB * 1024 = 16MB
#define DB_MAX_OUTPUT_SIZE 16384 // 16KB

#define DB_MAX_QUERY_OUTPUT_NUM_LVL1 100 // TOTAL: 1KB * 100 * 100 = 10MB * DB_MAX_DATA_POOL_SIZE
#define DB_MAX_QUERY_OUTPUT_NUM_LVL2 100
#define DB_MAX_QUERY_OUTPUT_SIZE 1024 // 1KB

// --------------- data struct --------------------
typedef struct db_param_require
{
    int type;
    void* value;
} db_param_require;

typedef struct db_param_release
{
    int type;
    void* value;
} db_param_release;

typedef struct db_param_version
{
    int major;
    int minor;
    int patch;
} db_param_version;

typedef struct db_param_init
{
    const char* option;

    int result;
} db_param_init;

typedef struct db_param_quit
{
    int result;
} db_param_quit;

#define DB_PARAM_EXEC 1
typedef struct db_param_exec
{
    const char* db_id;
    const char* sql;

    int result;
} db_param_exec;

#define DB_PARAM_QUERY 2
typedef struct db_param_query
{
    char*** out;
    size_t*** out_len;
    const char* db_id;
    const char* sql;

    int result;
} db_param_query;

// --------------- API --------------------
C_STYLE_EXPORT void db_version(sdk_context ctx);

C_STYLE_EXPORT void db_init(sdk_context ctx);

C_STYLE_EXPORT void db_quit(sdk_context ctx);

C_STYLE_EXPORT void db_require(sdk_context ctx);

C_STYLE_EXPORT void db_release(sdk_context ctx);

C_STYLE_EXPORT void db_exec(sdk_context ctx);

C_STYLE_EXPORT void db_query(sdk_context ctx);

#endif