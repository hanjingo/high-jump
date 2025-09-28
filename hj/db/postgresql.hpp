#ifndef POSTGRESQL_HPP
#define POSTGRESQL_HPP

#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <stdexcept>

namespace hj
{

class pg_connection
{
  public:
    explicit pg_connection(const std::string &options)
        : _conn(nullptr)
    {
        connect(options);
    }
    ~pg_connection() { disconnect(); }

    bool connect(const std::string &options)
    {
        disconnect();
        _conn = new pqxx::connection(options);
        if(!_conn->is_open())
        {
            delete _conn;
            _conn = nullptr;
            return false;
        }

        return true;
    }

    void disconnect()
    {
        if(_conn)
        {
            _conn->close();
            delete _conn;
            _conn = nullptr;
        }
    }

    void set_encoding(const std::string &encoding)
    {
        if(!is_open())
            return;

        _conn->set_client_encoding(encoding);
    }

    void exec(const std::string &sql)
    {
        pqxx::work txn(*_conn);
        txn.exec(sql);
        txn.commit();
    }

    std::vector<std::vector<std::string> > query(const std::string &sql)
    {
        pqxx::work                             txn(*_conn);
        pqxx::result                           res = txn.exec(sql);
        std::vector<std::vector<std::string> > rows;
        for(const auto &row : res)
        {
            std::vector<std::string> fields;
            for(const auto &field : row)
                fields.push_back(field.c_str());

            rows.push_back(fields);
        }
        return rows;
    }

    bool is_open() const { return _conn && _conn->is_open(); }

    pqxx::connection *native() { return _conn; }

  private:
    pqxx::connection *_conn;
};

} // namespace hj

#endif // POSTGRESQL_HPP