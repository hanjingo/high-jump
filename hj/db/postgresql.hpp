#ifndef POSTGRESQL_HPP
#define POSTGRESQL_HPP

#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>

namespace hj
{

class pg_connection
{
  public:
    explicit pg_connection(const std::string &options) { connect(options); }
    ~pg_connection() { disconnect(); }

    bool connect(const std::string &options) noexcept
    {
        disconnect();
        try
        {
            _conn = std::make_unique<pqxx::connection>(options);
            if(!_conn->is_open())
            {
                _conn.reset();
                return false;
            }
        }
        catch(...)
        {
            _conn.reset();
            return false;
        }

        return true;
    }

    void disconnect() noexcept
    {
        if(_conn)
        {
            try
            {
                _conn->close();
            }
            catch(...)
            {
            }
            _conn.reset();
        }
    }

    void set_encoding(const std::string &encoding) noexcept
    {
        if(!is_open())
            return;

        try
        {
            _conn->set_client_encoding(encoding);
        }
        catch(...)
        {
        }
    }

    bool exec(const std::string &sql) noexcept
    {
        if(!is_open())
            return false;

        try
        {
            pqxx::work txn(*_conn);
            txn.exec(sql);
            txn.commit();
            return true;
        }
        catch(...)
        {
            return false;
        }
    }

    std::vector<std::vector<std::string>> query(const std::string &sql) noexcept
    {
        std::vector<std::vector<std::string>> rows;
        if(!is_open())
            return rows;

        try
        {
            pqxx::work   txn(*_conn);
            pqxx::result res = txn.exec(sql);
            for(const auto &row : res)
            {
                std::vector<std::string> fields;
                for(const auto &field : row)
                    fields.push_back(field.c_str());
                rows.push_back(fields);
            }
        }
        catch(...)
        {
        }
        return rows;
    }

    bool is_open() const { return _conn && _conn->is_open(); }

    pqxx::connection *native() { return _conn.get(); }

  private:
    std::unique_ptr<pqxx::connection> _conn;
};

} // namespace hj

#endif // POSTGRESQL_HPP