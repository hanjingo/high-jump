#ifndef FIX_HPP
#define FIX_HPP

// // hffix: https://github.com/jamesdbrock/hffix
// #include <hffix.hpp>
// #include <string>
// #include <vector>
// #include <cstdint>
// #include <cstring>
// #include <stdexcept>

// namespace libcpp
// {

// // Simple wrapper for hffix FIX message building and parsing
// class fix_message {
// public:
//     fix_message() : buffer_(1024), writer_(&buffer_[0], &buffer_[0] +
//     buffer_.size()) {}

//     // Start a new FIX message
//     void start_message(uint8_t begin_string_major = 4, uint8_t
//     begin_string_minor = 2) {
//         writer_.reset(&buffer_[0], &buffer_[0] + buffer_.size());
//         writer_.push_back_begin_string(begin_string_major,
//         begin_string_minor); writer_.push_back_msgtype("D"); // Default to
//         NewOrderSingle
//     }

//     // Add a field to the message
//     template<typename T>
//     void add_field(int tag, const T& value) {
//         writer_.push_back(tag, value);
//     }

//     // Complete the message and get the raw FIX buffer
//     std::string finish_message() {
//         writer_.push_back_checksum();
//         return std::string(writer_.message_begin(), writer_.message_end());
//     }

//     // Parse a FIX message from string
//     void parse(const std::string& fix_str) {
//         message_ = hffix::message(fix_str.data(), fix_str.data() +
//         fix_str.size());
//     }

//     // Get field value by tag as string
//     std::string get_field(int tag) const {
//         if (!message_.is_valid()) return "";
//         for (hffix::field_iterator i = message_.begin(); i != message_.end();
//         ++i) {
//             if (i->tag() == tag) {
//                 return std::string(i->value().begin(), i->value().end());
//             }
//         }
//         return "";
//     }

//     // Get the underlying hffix::message
//     const hffix::message& raw_message() const { return message_; }

// private:
//     std::vector<char> buffer_;
//     hffix::message_writer writer_;
//     hffix::message message_;
// };

// } // namespace libcpp

#endif  // FIX_HPP