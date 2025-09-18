#ifndef SQLITE_HPP
#define SQLITE_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include <stdexcept>

namespace libcpp 
{

class sqlite 
{
public:
    typedef int(*exec_cb)(void* in, int argc, char** argv, char** col_name);

public:
	sqlite() 
        : _db{nullptr}
    {
    }
	~sqlite() 
    { 
        close(); 
    }

    inline bool is_open() const { return _db != nullptr; }

	bool open(const std::string& filename) 
    {
		close();
		return sqlite3_open(filename.c_str(), &_db) == SQLITE_OK;
	}

	void close() 
    {
		if (!_db)
            return;

        sqlite3_close(_db);
        _db = nullptr;
	}

	bool exec(const std::string& sql) 
    {
        if (!_db) 
			return false;

        char* errmsg = nullptr;
        int rc = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errmsg);
        if (rc != SQLITE_OK) 
		{
            if (errmsg) 
			{
                fprintf(stderr, "sqlite exec error: %s\n", errmsg);
                sqlite3_free(errmsg);
            }
            return false;
        }

        if (errmsg) 
			sqlite3_free(errmsg);
			
        return true;
	}

	// Query: returns vector of rows, each row is vector<string>
	std::vector<std::vector<std::string>> query(const std::string& sql, exec_cb callback) 
    {
		std::vector<std::vector<std::string>> rows;
		char* errmsg = nullptr;
		int rc = sqlite3_exec(_db, sql.c_str(), callback, &rows, &errmsg);
		if (rc != SQLITE_OK || errmsg) 
            sqlite3_free(errmsg);

		return rows;
	}

private:
    sqlite(const sqlite&) = delete;
    sqlite& operator=(const sqlite&) = delete;
    sqlite(sqlite&&) = delete;
    sqlite& operator=(sqlite&&) = delete;

private:
	sqlite3* _db;
};

} // namespace libcpp

#endif