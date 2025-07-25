#ifndef ZIP_HPP
#define ZIP_HPP

#include <zlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <ctime>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <iomanip>

// namespace libcpp 
// {

// namespace zip
// {

// class exception : public std::runtime_error 
// {
// public:
//     explicit exception(const std::string& message) 
//         : std::runtime_error("ZIP Error: " + message) {}
// };

// enum class compression_level 
// {
//     none = Z_NO_COMPRESSION,
//     fast = Z_BEST_SPEED,
//     best = Z_BEST_COMPRESSION,
//     default_level = Z_DEFAULT_COMPRESSION
// };

// struct local_file_header 
// {
//     std::uint32_t signature;
//     std::uint16_t version_needed;
//     std::uint16_t flags;
//     std::uint16_t compression_method;
//     std::uint16_t mod_time;
//     std::uint16_t mod_date;
//     std::uint32_t crc32;
//     std::uint32_t compressed_size;
//     std::uint32_t uncompressed_size;
//     std::uint16_t filename_length;
//     std::uint16_t extra_length;
// };

// struct central_directory_header 
// {
//     std::uint32_t signature;
//     std::uint16_t version_made;
//     std::uint16_t version_needed;
//     std::uint16_t flags;
//     std::uint16_t compression_method;
//     std::uint16_t mod_time;
//     std::uint16_t mod_date;
//     std::uint32_t crc32;
//     std::uint32_t compressed_size;
//     std::uint32_t uncompressed_size;
//     std::uint16_t filename_length;
//     std::uint16_t extra_length;
//     std::uint16_t comment_length;
//     std::uint16_t disk_start;
//     std::uint16_t internal_attr;
//     std::uint32_t external_attr;
//     std::uint32_t local_header_offset;
// };

// struct end_of_central_directory 
// {
//     std::uint32_t signature;
//     std::uint16_t disk_number;
//     std::uint16_t central_dir_disk;
//     std::uint16_t central_dir_entries;
//     std::uint16_t total_entries;
//     std::uint32_t central_dir_size;
//     std::uint32_t central_dir_offset;
//     std::uint16_t comment_length;
// };

// struct file_info 
// {
//     std::string filename;
//     std::uint32_t uncompressed_size;
//     std::uint32_t compressed_size;
//     std::time_t modified_time;
//     std::uint32_t crc32;
//     std::uint16_t compression_method;
//     bool is_directory;
    
//     file_info() 
//         : uncompressed_size(0)
//         , compressed_size(0)
//         , modified_time(0)
//         , crc32(0)
//         , compression_method(Z_DEFLATED)
//         , is_directory(false) 
//     {}
// };

// static std::uint32_t crc32(const void* data, std::size_t size);
// static std::time_t dos_time_to_time_t(std::uint16_t dos_time, std::uint16_t dos_date);

// class writer 
// {
// private:
//     std::ofstream file_;
//     std::string filename_;
//     compression_level level_;
//     std::vector<file_info> entries_;
//     std::vector<std::uint32_t> local_header_offsets_;
    
// public:
//     explicit writer(const std::string& filename, 
//                     compression_level level = compression_level::default_level)
//         : filename_(filename), level_(level) {
        
//         file_.open(filename, std::ios::binary);
//         if (!file_.is_open()) 
//             throw zip_exception("Failed to create ZIP file: " + filename);
//     }
    
//     ~writer() {
//         if (file_.is_open()) 
//         {
//             try {
//                 close();
//             } catch (...) {
//             }
//         }
//     }
    
//     writer(const writer&) = delete;
//     writer& operator=(const writer&) = delete;
    
//     writer(writer&& other) noexcept 
//         : file_(std::move(other.file_))
//         , filename_(std::move(other.filename_))
//         , level_(other.level_)
//         , entries_(std::move(other.entries_))
//         , local_header_offsets_(std::move(other.local_header_offsets_)) 
//     {
//     }
    
//     writer& operator=(writer&& other) noexcept 
//     {
//         if (this != &other) {
//             if (file_.is_open()) {
//                 try {
//                     close();
//                 } catch (...) {}
//             }
//             file_ = std::move(other.file_);
//             filename_ = std::move(other.filename_);
//             level_ = other.level_;
//             entries_ = std::move(other.entries_);
//             local_header_offsets_ = std::move(other.local_header_offsets_);
//         }
//         return *this;
//     }
    
//     void add_file(const std::string& file_path, const std::string& archive_name = "") 
//     {
//         std::ifstream input(file_path, std::ios::binary);
//         if (!input.is_open())
//             throw zip_exception("Failed to open file: " + file_path);
        
//         std::vector<std::uint8_t> data;
//         input.seekg(0, std::ios::end);
//         data.resize(input.tellg());
//         input.seekg(0, std::ios::beg);
//         input.read(reinterpret_cast<char*>(data.data()), data.size());
        
//         std::string name = archive_name.empty() ? 
//             std::filesystem::path(file_path).filename().string() : archive_name;
        
//         std::time_t file_time = get_file_time(file_path);
//         add_data(name, data.data(), data.size(), file_time);
//     }
    
//     void add_data(const std::string& archive_name, const void* data, std::size_t size, 
//                   std::time_t mod_time = 0) 
//     {
//         if (!file_.is_open())
//             throw exception("ZIP file is not open");
        
//         if (mod_time == 0)
//             mod_time = std::time(nullptr);
        
//         local_header_offsets_.push_back(static_cast<std::uint32_t>(file_.tellp()));
        
//         std::uint32_t crc = crc32(data, size);
        
//         std::vector<std::uint8_t> compressed_data;
//         std::uint16_t compression_method = Z_NO_COMPRESSION;
        
//         if (level_ != compression_level::none && size > 0) 
//         {
//             compressed_data = compress_data(data, size);
//             if (compressed_data.size() < size)
//                 compression_method = Z_DEFLATED;
//             else
//                 compressed_data.clear();
//         }
        
//         if (compressed_data.empty()) 
//         {
//             compressed_data.assign(
//                 static_cast<const std::uint8_t*>(data),
//                 static_cast<const std::uint8_t*>(data) + size
//             );
//             compression_method = Z_NO_COMPRESSION;
//         }
        
//         write_local_file_header(archive_name, compression_method, mod_time, 
//                                 crc, compressed_data.size(), size);
        
//         file_.write(reinterpret_cast<const char*>(compressed_data.data()), 
//                     compressed_data.size());
        
//         file_info info;
//         info.filename = archive_name;
//         info.uncompressed_size = static_cast<std::uint32_t>(size);
//         info.compressed_size = static_cast<std::uint32_t>(compressed_data.size());
//         info.modified_time = mod_time;
//         info.crc32 = crc;
//         info.compression_method = compression_method;
//         info.is_directory = archive_name.back() == '/';
        
//         entries_.push_back(info);
//     }
    
//     // 添加字符串
//     void add_string(const std::string& archive_name, const std::string& content) {
//         add_data(archive_name, content.data(), content.size());
//     }
    
//     // 添加目录
//     void add_directory(const std::string& dir_name) {
//         std::string name = dir_name;
//         if (!name.empty() && name.back() != '/') {
//             name += '/';
//         }
//         add_data(name, nullptr, 0);
//     }
    
//     // 递归添加目录
//     void add_directory_recursive(const std::string& dir_path, const std::string& base_name = "") {
//         namespace fs = std::filesystem;
        
//         if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
//             throw zip_exception("Directory does not exist: " + dir_path);
//         }
        
//         std::string base = base_name.empty() ? 
//             fs::path(dir_path).filename().string() : base_name;
        
//         // 添加目录本身
//         add_directory(base);
        
//         // 递归添加内容
//         for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
//             std::string relative_path = base + "/" + 
//                 fs::relative(entry.path(), dir_path).string();
            
//             // 替换反斜杠为正斜杠（ZIP标准）
//             std::replace(relative_path.begin(), relative_path.end(), '\\', '/');
            
//             if (entry.is_directory()) {
//                 add_directory(relative_path);
//             } else if (entry.is_regular_file()) {
//                 add_file(entry.path().string(), relative_path);
//             }
//         }
//     }
    
//     // 关闭ZIP文件
//     void close() {
//         if (!file_.is_open()) {
//             return;
//         }
        
//         // 写入中央目录
//         std::uint32_t central_dir_offset = static_cast<std::uint32_t>(file_.tellp());
//         write_central_directory();
        
//         // 写入中央目录结束记录
//         std::uint32_t central_dir_size = static_cast<std::uint32_t>(file_.tellp()) - central_dir_offset;
//         write_end_of_central_directory(central_dir_offset, central_dir_size);
        
//         file_.close();
//     }
    
// private:
//     std::vector<std::uint8_t> compress_data(const void* data, std::size_t size) {
//         z_stream stream = {};
        
//         if (deflateInit(&stream, static_cast<int>(level_)) != Z_OK) {
//             throw zip_exception("Failed to initialize compression");
//         }
        
//         stream.next_in = static_cast<Bytef*>(const_cast<void*>(data));
//         stream.avail_in = static_cast<uInt>(size);
        
//         std::vector<std::uint8_t> compressed;
//         std::uint8_t buffer[8192];
        
//         do {
//             stream.next_out = buffer;
//             stream.avail_out = sizeof(buffer);
            
//             int result = deflate(&stream, Z_FINISH);
            
//             if (result != Z_STREAM_END && result != Z_OK) {
//                 deflateEnd(&stream);
//                 throw zip_exception("Compression failed");
//             }
            
//             std::size_t bytes_written = sizeof(buffer) - stream.avail_out;
//             compressed.insert(compressed.end(), buffer, buffer + bytes_written);
            
//         } while (stream.avail_out == 0);
        
//         deflateEnd(&stream);
//         return compressed;
//     }
    
//     void write_local_file_header(const std::string& filename, std::uint16_t compression_method,
//                                 std::time_t mod_time, std::uint32_t crc32,
//                                 std::uint32_t compressed_size, std::uint32_t uncompressed_size) {
//         binary_writer writer(file_);
        
//         auto [dos_time, dos_date] = time_utils::to_dos_time(mod_time);
        
//         writer.write_uint32(0x04034b50); // 本地文件头签名
//         writer.write_uint16(20);          // 版本需求
//         writer.write_uint16(0);           // 通用位标志
//         writer.write_uint16(compression_method);
//         writer.write_uint16(dos_time);
//         writer.write_uint16(dos_date);
//         writer.write_uint32(crc32);
//         writer.write_uint32(compressed_size);
//         writer.write_uint32(uncompressed_size);
//         writer.write_uint16(static_cast<std::uint16_t>(filename.length()));
//         writer.write_uint16(0);           // 扩展字段长度
//         writer.write_string(filename);
//     }
    
//     void write_central_directory() {
//         binary_writer writer(file_);
        
//         for (std::size_t i = 0; i < entries_.size(); ++i) {
//             const auto& entry = entries_[i];
//             auto [dos_time, dos_date] = time_utils::to_dos_time(entry.modified_time);
            
//             writer.write_uint32(0x02014b50); // 中央目录签名
//             writer.write_uint16(20);          // 创建版本
//             writer.write_uint16(20);          // 版本需求
//             writer.write_uint16(0);           // 通用位标志
//             writer.write_uint16(entry.compression_method);
//             writer.write_uint16(dos_time);
//             writer.write_uint16(dos_date);
//             writer.write_uint32(entry.crc32);
//             writer.write_uint32(entry.compressed_size);
//             writer.write_uint32(entry.uncompressed_size);
//             writer.write_uint16(static_cast<std::uint16_t>(entry.filename.length()));
//             writer.write_uint16(0);           // 扩展字段长度
//             writer.write_uint16(0);           // 注释长度
//             writer.write_uint16(0);           // 起始磁盘号
//             writer.write_uint16(0);           // 内部文件属性
//             writer.write_uint32(entry.is_directory ? 0x10 : 0); // 外部文件属性
//             writer.write_uint32(local_header_offsets_[i]);
//             writer.write_string(entry.filename);
//         }
//     }
    
//     void write_end_of_central_directory(std::uint32_t central_dir_offset, 
//                                        std::uint32_t central_dir_size) {
//         binary_writer writer(file_);
        
//         writer.write_uint32(0x06054b50); // 结束记录签名
//         writer.write_uint16(0);           // 当前磁盘号
//         writer.write_uint16(0);           // 中央目录起始磁盘号
//         writer.write_uint16(static_cast<std::uint16_t>(entries_.size()));
//         writer.write_uint16(static_cast<std::uint16_t>(entries_.size()));
//         writer.write_uint32(central_dir_size);
//         writer.write_uint32(central_dir_offset);
//         writer.write_uint16(0);           // 注释长度
//     }
    
//     std::time_t get_file_time(const std::string& file_path) {
//         namespace fs = std::filesystem;
//         try {
//             auto ftime = fs::last_write_time(file_path);
//             auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
//                 ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
//             return std::chrono::system_clock::to_time_t(sctp);
//         } catch (...) {
//             return std::time(nullptr);
//         }
//     }
// };

// // ZIP 读取器类
// class zip_reader {
// private:
//     std::ifstream file_;
//     std::string filename_;
//     std::vector<zip_file_info> entries_;
//     std::vector<std::uint32_t> local_header_offsets_;
    
// public:
//     explicit zip_reader(const std::string& filename) : filename_(filename) {
//         file_.open(filename, std::ios::binary);
//         if (!file_.is_open()) {
//             throw zip_exception("Failed to open ZIP file: " + filename);
//         }
        
//         read_central_directory();
//     }
    
//     ~zip_reader() {
//         if (file_.is_open()) {
//             file_.close();
//         }
//     }
    
//     // 禁用拷贝构造和赋值
//     zip_reader(const zip_reader&) = delete;
//     zip_reader& operator=(const zip_reader&) = delete;
    
//     // 支持移动构造和赋值
//     zip_reader(zip_reader&& other) noexcept 
//         : file_(std::move(other.file_))
//         , filename_(std::move(other.filename_))
//         , entries_(std::move(other.entries_))
//         , local_header_offsets_(std::move(other.local_header_offsets_)) {
//     }
    
//     zip_reader& operator=(zip_reader&& other) noexcept {
//         if (this != &other) {
//             if (file_.is_open()) {
//                 file_.close();
//             }
//             file_ = std::move(other.file_);
//             filename_ = std::move(other.filename_);
//             entries_ = std::move(other.entries_);
//             local_header_offsets_ = std::move(other.local_header_offsets_);
//         }
//         return *this;
//     }
    
//     // 获取文件列表
//     const std::vector<zip_file_info>& get_file_list() const {
//         return entries_;
//     }
    
//     // 提取文件到内存
//     std::vector<std::uint8_t> extract_to_memory(const std::string& archive_name) {
//         auto it = std::find_if(entries_.begin(), entries_.end(),
//             [&archive_name](const zip_file_info& info) {
//                 return info.filename == archive_name;
//             });
        
//         if (it == entries_.end()) {
//             throw zip_exception("File not found in ZIP: " + archive_name);
//         }
        
//         std::size_t index = std::distance(entries_.begin(), it);
//         return extract_file_data(index);
//     }
    
//     // 提取字符串
//     std::string extract_string(const std::string& archive_name) {
//         auto data = extract_to_memory(archive_name);
//         return std::string(data.begin(), data.end());
//     }
    
//     // 提取文件到磁盘
//     void extract_file(const std::string& archive_name, const std::string& output_path) {
//         auto data = extract_to_memory(archive_name);
        
//         // 创建输出目录
//         namespace fs = std::filesystem;
//         fs::path output_file_path(output_path);
//         if (output_file_path.has_parent_path()) {
//             fs::create_directories(output_file_path.parent_path());
//         }
        
//         // 检查是否是目录
//         if (archive_name.back() == '/') {
//             fs::create_directories(output_path);
//             return;
//         }
        
//         std::ofstream output(output_path, std::ios::binary);
//         if (!output.is_open()) {
//             throw zip_exception("Failed to create output file: " + output_path);
//         }
        
//         output.write(reinterpret_cast<const char*>(data.data()), data.size());
//     }
    
//     // 提取所有文件
//     void extract_all(const std::string& output_dir) {
//         for (const auto& entry : entries_) {
//             namespace fs = std::filesystem;
//             std::string output_path = (fs::path(output_dir) / entry.filename).string();
            
//             // 替换正斜杠为平台特定的分隔符
//             std::replace(output_path.begin(), output_path.end(), '/', fs::path::preferred_separator);
            
//             extract_file(entry.filename, output_path);
//         }
//     }
    
//     // 检查文件是否存在
//     bool has_file(const std::string& archive_name) const {
//         return std::find_if(entries_.begin(), entries_.end(),
//             [&archive_name](const zip_file_info& info) {
//                 return info.filename == archive_name;
//             }) != entries_.end();
//     }
    
//     // 获取文件数量
//     std::size_t get_file_count() const {
//         return entries_.size();
//     }
    
// private:
//     void read_central_directory() {
//         // 查找中央目录结束记录
//         zip_end_of_central_directory eocd = find_end_of_central_directory();
        
//         // 读取中央目录
//         file_.seekg(eocd.central_dir_offset, std::ios::beg);
//         binary_reader reader(file_);
        
//         entries_.clear();
//         local_header_offsets_.clear();
        
//         for (std::uint16_t i = 0; i < eocd.total_entries; ++i) {
//             zip_file_info info;
//             std::uint32_t signature = reader.read_uint32();
            
//             if (signature != 0x02014b50) {
//                 throw zip_exception("Invalid central directory signature");
//             }
            
//             reader.read_uint16(); // version_made
//             reader.read_uint16(); // version_needed
//             reader.read_uint16(); // flags
//             info.compression_method = reader.read_uint16();
//             std::uint16_t dos_time = reader.read_uint16();
//             std::uint16_t dos_date = reader.read_uint16();
//             info.crc32 = reader.read_uint32();
//             info.compressed_size = reader.read_uint32();
//             info.uncompressed_size = reader.read_uint32();
//             std::uint16_t filename_length = reader.read_uint16();
//             std::uint16_t extra_length = reader.read_uint16();
//             std::uint16_t comment_length = reader.read_uint16();
//             reader.read_uint16(); // disk_start
//             reader.read_uint16(); // internal_attr
//             reader.read_uint32(); // external_attr
//             std::uint32_t local_header_offset = reader.read_uint32();
            
//             info.filename = reader.read_string(filename_length);
//             info.modified_time = time_utils::from_dos_time(dos_time, dos_date);
//             info.is_directory = info.filename.back() == '/';
            
//             // 跳过扩展字段和注释
//             file_.seekg(extra_length + comment_length, std::ios::cur);
            
//             entries_.push_back(info);
//             local_header_offsets_.push_back(local_header_offset);
//         }
//     }
    
//     zip_end_of_central_directory find_end_of_central_directory() {
//         // 从文件末尾开始搜索 EOCD 记录
//         file_.seekg(0, std::ios::end);
//         std::streamsize file_size = file_.tellg();
        
//         // EOCD 记录最小为 22 字节
//         std::streamsize search_start = std::max(std::streamsize(0), file_size - 65557); // 最大注释长度 65535 + 22
//         file_.seekg(search_start, std::ios::beg);
        
//         std::vector<char> buffer(static_cast<std::size_t>(file_size - search_start));
//         file_.read(buffer.data(), buffer.size());
        
//         // 搜索 EOCD 签名
//         for (std::size_t i = buffer.size() - 22; i != SIZE_MAX; --i) {
//             if (buffer[i] == 0x50 && buffer[i+1] == 0x4b && 
//                 buffer[i+2] == 0x05 && buffer[i+3] == 0x06) {
                
//                 // 找到 EOCD 记录
//                 file_.seekg(search_start + i, std::ios::beg);
//                 binary_reader reader(file_);
                
//                 zip_end_of_central_directory eocd;
//                 eocd.signature = reader.read_uint32();
//                 eocd.disk_number = reader.read_uint16();
//                 eocd.central_dir_disk = reader.read_uint16();
//                 eocd.central_dir_entries = reader.read_uint16();
//                 eocd.total_entries = reader.read_uint16();
//                 eocd.central_dir_size = reader.read_uint32();
//                 eocd.central_dir_offset = reader.read_uint32();
//                 eocd.comment_length = reader.read_uint16();
                
//                 return eocd;
//             }
//         }
        
//         throw zip_exception("End of central directory record not found");
//     }
    
//     std::vector<std::uint8_t> extract_file_data(std::size_t index) {
//         if (index >= entries_.size()) {
//             throw zip_exception("Invalid file index");
//         }
        
//         const auto& entry = entries_[index];
//         std::uint32_t offset = local_header_offsets_[index];
        
//         // 跳转到本地文件头
//         file_.seekg(offset, std::ios::beg);
//         binary_reader reader(file_);
        
//         // 读取本地文件头
//         std::uint32_t signature = reader.read_uint32();
//         if (signature != 0x04034b50) {
//             throw zip_exception("Invalid local file header signature");
//         }
        
//         reader.read_uint16(); // version_needed
//         reader.read_uint16(); // flags
//         reader.read_uint16(); // compression_method
//         reader.read_uint16(); // mod_time
//         reader.read_uint16(); // mod_date
//         reader.read_uint32(); // crc32
//         reader.read_uint32(); // compressed_size
//         reader.read_uint32(); // uncompressed_size
//         std::uint16_t filename_length = reader.read_uint16();
//         std::uint16_t extra_length = reader.read_uint16();
        
//         // 跳过文件名和扩展字段
//         file_.seekg(filename_length + extra_length, std::ios::cur);
        
//         // 读取压缩数据
//         std::vector<std::uint8_t> compressed_data(entry.compressed_size);
//         file_.read(reinterpret_cast<char*>(compressed_data.data()), compressed_data.size());
        
//         // 解压缩数据
//         if (entry.compression_method == Z_DEFLATED) {
//             return decompress_data(compressed_data, entry.uncompressed_size);
//         } else {
//             return compressed_data; // 无压缩
//         }
//     }
    
//     std::vector<std::uint8_t> decompress_data(const std::vector<std::uint8_t>& compressed_data,
//                                              std::uint32_t uncompressed_size) {
//         z_stream stream = {};
        
//         if (inflateInit(&stream) != Z_OK) {
//             throw zip_exception("Failed to initialize decompression");
//         }
        
//         stream.next_in = const_cast<Bytef*>(compressed_data.data());
//         stream.avail_in = static_cast<uInt>(compressed_data.size());
        
//         std::vector<std::uint8_t> decompressed(uncompressed_size);
//         stream.next_out = decompressed.data();
//         stream.avail_out = static_cast<uInt>(decompressed.size());
        
//         int result = inflate(&stream, Z_FINISH);
        
//         inflateEnd(&stream);
        
//         if (result != Z_STREAM_END) {
//             throw zip_exception("Decompression failed");
//         }
        
//         return decompressed;
//     }
// };

// // 便利函数
// namespace zip {
//     // 压缩单个文件
//     inline void compress_file(const std::string& input_file, const std::string& output_zip,
//                              compression_level level = compression_level::default_level) {
//         zip_writer writer(output_zip, level);
//         writer.add_file(input_file);
//         writer.close();
//     }
    
//     // 压缩目录
//     inline void compress_directory(const std::string& input_dir, const std::string& output_zip,
//                                   compression_level level = compression_level::default_level) {
//         zip_writer writer(output_zip, level);
//         writer.add_directory_recursive(input_dir);
//         writer.close();
//     }
    
//     // 压缩多个文件
//     inline void compress_files(const std::vector<std::string>& input_files, 
//                               const std::string& output_zip,
//                               compression_level level = compression_level::default_level) {
//         zip_writer writer(output_zip, level);
//         for (const auto& file : input_files) {
//             writer.add_file(file);
//         }
//         writer.close();
//     }
    
//     // 解压缩ZIP文件
//     inline void extract(const std::string& zip_file, const std::string& output_dir) {
//         zip_reader reader(zip_file);
//         reader.extract_all(output_dir);
//     }
    
//     // 创建ZIP文件包含字符串数据
//     inline void create_from_strings(const std::string& output_zip,
//                                    const std::vector<std::pair<std::string, std::string>>& files,
//                                    compression_level level = compression_level::default_level) {
//         zip_writer writer(output_zip, level);
//         for (const auto& file : files) {
//             writer.add_string(file.first, file.second);
//         }
//         writer.close();
//     }
    
//     // 读取ZIP文件中的所有字符串
//     inline std::vector<std::pair<std::string, std::string>> read_all_strings(const std::string& zip_file) {
//         zip_reader reader(zip_file);
//         const auto& files = reader.get_file_list();
        
//         std::vector<std::pair<std::string, std::string>> result;
//         for (const auto& file : files) {
//             if (!file.is_directory) {
//                 try {
//                     std::string content = reader.extract_string(file.filename);
//                     result.emplace_back(file.filename, content);
//                 } catch (const zip_exception&) {
//                     // 跳过无法读取的文件
//                 }
//             }
//         }
        
//         return result;
//     }
    
//     // 获取ZIP文件信息
//     inline std::vector<zip_file_info> list_files(const std::string& zip_file) {
//         zip_reader reader(zip_file);
//         return reader.get_file_list();
//     }
// }

// } // namespace zip

// } // namespace libcpp

#endif // ZIP_HPP