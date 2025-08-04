#ifndef POSTGRESQL_HPP
#define POSTGRESQL_HPP

// #include <pqxx/pqxx>
// #include <string>
// #include <vector>
// #include <stdexcept>

// namespace libcpp
// {

// // Simple wrapper for PostgreSQL database using libpqxx
// class postgresql
// {
// public:
//     // Construct with connection string
//     explicit postgresql(const std::string& conninfo)
//         : conn_(nullptr)
//     {
//         connect(conninfo);
//     }

//     // Destructor closes connection
//     ~postgresql() {
//         disconnect();
//     }

//     // Connect to database
//     void connect(const std::string& conninfo) {
//         disconnect();
//         conn_ = new pqxx::connection(conninfo);
//         if (!conn_->is_open())
//             throw std::runtime_error("Failed to open PostgreSQL connection");
//     }

//     // Disconnect from database
//     void disconnect() {
//         if (conn_) {
//             conn_->disconnect();
//             delete conn_;
//             conn_ = nullptr;
//         }
//     }

//     // Execute a SQL command (no result)
//     void exec(const std::string& sql) {
//         pqxx::work txn(*conn_);
//         txn.exec(sql);
//         txn.commit();
//     }

//     // Execute a SQL query and return results as vector of vector of strings
//     std::vector<std::vector<std::string>> query(const std::string& sql) {
//         pqxx::work txn(*conn_);
//         pqxx::result res = txn.exec(sql);
//         std::vector<std::vector<std::string>> rows;
//         for (const auto& row : res) {
//             std::vector<std::string> fields;
//             for (const auto& field : row) {
//                 fields.push_back(field.c_str());
//             }
//             rows.push_back(fields);
//         }
//         return rows;
//     }

//     // Check if connection is open
//     bool is_open() const {
//         return conn_ && conn_->is_open();
//     }

//     // Get underlying pqxx::connection
//     pqxx::connection* native() { return conn_; }

// private:
//     pqxx::connection* conn_;
// };

// } // namespace libcpp

#endif  // POSTGRESQL_HPP