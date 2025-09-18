#ifndef DB_CORE_API_H
#define DB_CORE_API_H

#include <libcpp/os/dll.h>

typedef struct db_context
{
    unsigned long sz;
    void* user_data;
    void (*cb)(void*);
} db_context;

typedef void(*db_callback)(void*);
typedef void(*db_api)(db_context);

C_STYLE_EXPORT void db_version(db_context ctx);

C_STYLE_EXPORT void db_init(db_context ctx);

C_STYLE_EXPORT void db_quit(db_context ctx);

// add your code here...
// ---------------- error code [1000, 1999] --------------------
#define DB_OK                                  0
#define DB_ERR_FAIL                            1001
#define DB_ERR_CORE_NOT_LOAD                   1002
#define DB_ERR_CORE_LOAD_FAIL                  1003
#define DB_ERR_VERSION_MISMATCH                1004
#define DB_ERR_INIT_FAIL                       1005
#define DB_ERR_QUIT_FAIL                       1006

#define DB_ERR_SQLITE_EXEC_FAIL                1100
#define DB_ERR_SQLITE_NOT_OPEN                 1101
#define DB_ERR_SQLITE_ALREADY_OPEN             1102
#define DB_ERR_SQLITE_OPEN_FAIL                1103
#define DB_ERR_SQLITE_GET_CONN_FAIL            1104

#define DB_ERR_END                             1999

// --------------- const --------------------

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
    int sqlite_conn_capa;
    const char* sqlite_db_path;

    int result;
} db_param_init;

typedef struct db_param_quit
{
    int result;
} db_param_quit;

typedef struct db_param_exec
{
    const char* db_type; // e.g. "sqlite"
    const char* sql;
    
    int result;
} db_param_exec;

// --------------- API --------------------
C_STYLE_EXPORT void db_exec(db_context ctx);

#endif