# High-Jump 模块文档

本文档详细介绍了High-Jump C++17库框架中所有模块的功能特性。

## 概述

High-Jump是一个为高性能应用程序设计的综合性C++17库框架。它由18个核心模块组成，每个模块专注于特定的功能领域。

---

## 模块详情

### algo - 算法与数据结构
**用途**: 高效计算的通用算法和数据结构

**核心功能**:
- **跳表**: 支持快速搜索、插入和删除的概率性数据结构
- **布隆过滤器**: 用于成员测试的空间高效概率性数据结构
- **状态机**: 用于工作流管理的有限状态机实现
- **行为树**: 游戏开发和机器人的AI行为建模
- **排序算法**: 各种优化的排序算法实现
- **图算法**: 路径查找、遍历和图分析工具

**使用场景**: 游戏AI、数据处理、搜索引擎、工作流系统

---

### compress - 压缩工具
**用途**: 支持多种格式的数据压缩和解压缩

**核心功能**:
- **GZIP支持**: 标准GZIP压缩/解压缩
- **流处理**: 大数据集的高效流式压缩
- **内存管理**: 压缩操作的优化内存使用
- **格式检测**: 自动格式检测和处理
- **压缩级别**: 可配置的压缩比与速度权衡

**使用场景**: 文件归档、网络数据传输、存储优化

---

### crypto - 密码学
**用途**: 全面的密码学算法和安全功能

**核心功能**:
- **对称加密**: AES、DES多模式实现
- **非对称加密**: RSA公钥密码学支持
- **哈希函数**: MD5、SHA系列（SHA-1、SHA-256、SHA-512）
- **编码**: Base64编码/解码工具
- **密钥管理**: 安全密钥生成和管理
- **数字签名**: 消息认证和完整性验证

**使用场景**: 安全通信、数据保护、认证系统

---

### db - 数据库支持
**用途**: 数据库连接和操作，支持连接池

**核心功能**:
- **多数据库支持**: SQLite、Redis、PostgreSQL、ClickHouse
- **连接池**: 高效的连接管理和重用
- **查询构建器**: 类型安全的SQL查询构造
- **事务管理**: ACID事务支持
- **ORM特性**: 对象关系映射功能
- **异步操作**: 非阻塞数据库操作

**使用场景**: Web应用程序、数据分析、缓存系统、企业软件

---

### encoding - 编码与解码
**用途**: 用于数据转换的各种编码/解码格式

**核心功能**:
- **文本编码**: Hex、Unicode、UTF-8/16/32支持
- **数据格式**: XML、JSON、YAML、Protocol Buffers
- **二进制编码**: 高效的二进制序列化
- **字符集转换**: 多语言字符支持
- **验证**: 格式验证和错误处理
- **流支持**: 大文件处理能力

**使用场景**: 数据交换、配置文件、国际化

---

### hardware - 硬件信息与控制
**用途**: 系统硬件检测和控制功能

**核心功能**:
- **CPU信息**: 核心数、频率、架构检测
- **GPU支持**: 显卡信息和基本控制
- **内存管理**: RAM使用监控和优化
- **存储设备**: 磁盘信息、使用统计
- **USB设备**: 设备枚举和基本通信
- **传感器**: 温度、电源和性能监控

**使用场景**: 系统监控、性能优化、硬件诊断

---

### io - I/O操作
**用途**: 文件操作、缓冲区管理和异步I/O

**核心功能**:
- **文件操作**: 支持Unicode的跨平台文件处理
- **缓冲区管理**: 高效的内存缓冲区操作
- **异步I/O**: 支持回调的非阻塞I/O操作
- **任务处理**: 工作队列和任务调度
- **流处理**: 数据流处理和转换
- **目录操作**: 递归目录遍历和操作

**使用场景**: 文件处理、数据管道、服务器应用程序

---

### log - 日志系统
**用途**: 基于spdlog的高性能日志系统

**核心功能**:
- **多种输出**: 控制台、文件、轮转文件、网络日志
- **日志级别**: Trace、Debug、Info、Warning、Error、Critical
- **格式化**: 可自定义的日志消息格式
- **线程安全**: 多线程日志支持
- **性能**: 最小开销的异步日志
- **过滤**: 基于日志级别和模式的过滤

**使用场景**: 应用程序调试、系统监控、审计跟踪

---

### math - 数学与线性代数
**用途**: 数学运算、矩阵和数值计算

**核心功能**:
- **矩阵运算**: 优化实现的线性代数
- **随机数生成**: 多种RNG算法和分布
- **向量数学**: 2D/3D向量运算和变换
- **统计函数**: 均值、方差、相关性、回归
- **数值方法**: 积分、微分、优化
- **几何算法**: 碰撞检测、几何变换

**使用场景**: 科学计算、游戏开发、数据分析、仿真

---

### misc - 杂项工具
**用途**: 各种实用功能和专用工具

**核心功能**:
- **错误处理**: 综合错误码系统
- **回调管理**: 函数回调和事件处理
- **哈希表**: 高性能哈希表实现
- **PDF生成**: 文档创建和操作
- **gRPC支持**: 远程过程调用框架集成
- **ZeroMQ**: 高性能消息库集成

**使用场景**: 错误报告、事件系统、文档生成、分布式系统

---

### net - 网络通信
**用途**: 网络协议和通信框架

**核心功能**:
- **TCP/UDP**: 底层套接字编程抽象
- **HTTP客户端/服务器**: 支持SSL/TLS的RESTful API
- **WebSocket**: 实时双向通信
- **ZeroMQ集成**: 高性能消息模式
- **协议处理**: 自定义协议开发支持
- **SSL/TLS**: 安全通信加密

**使用场景**: Web服务、实时应用程序、分布式系统、IoT

---

### os - 操作系统接口
**用途**: 跨平台OS特定功能

**核心功能**:
- **进程管理**: 进程创建、监控和控制
- **信号处理**: Unix信号处理和Windows事件处理
- **环境变量**: 跨平台环境管理
- **系统信息**: OS版本、架构、功能
- **兼容层**: 便携式代码的平台抽象
- **资源管理**: 系统资源监控和控制

**使用场景**: 系统管理、跨平台应用程序、监控工具

---

### sync - 同步与线程
**用途**: 线程安全工具和并发编程

**核心功能**:
- **通道通信**: Go风格的线程间通道通信
- **一次执行**: 确保跨线程的单次执行
- **线程安全容器**: 并发数据结构
- **协程**: 无栈协程实现
- **原子操作**: 无锁编程原语
- **同步原语**: 互斥锁、信号量、条件变量

**使用场景**: 并发应用程序、并行处理、服务器开发

---

### testing - 测试与异常处理
**用途**: 综合测试框架和崩溃处理

**核心功能**:
- **Google Test集成**: 完整的GTest框架支持
- **Breakpad集成**: 崩溃转储生成和分析
- **单元测试**: 全面的测试用例管理
- **Mock支持**: 隔离测试的对象模拟
- **性能测试**: 性能测试的基准集成
- **异常处理**: 跨平台的结构化异常处理

**使用场景**: 质量保证、调试、性能验证

---

### time - 日期与时间处理
**用途**: 全面的日期、时间和定时器功能

**核心功能**:
- **日期/时间操作**: 解析、格式化、算术运算
- **时区支持**: 跨平台时区处理
- **高精度定时器**: 微秒精度计时
- **持续时间计算**: 时间间隔计算
- **日历函数**: 日期计算和日历操作
- **秒表工具**: 性能计时和测量

**使用场景**: 调度系统、性能测量、基于时间的应用程序

---

### types - 通用数据类型
**用途**: 增强的数据类型和类型工具

**核心功能**:
- **Any类型**: 任何值类型的类型安全容器
- **字节工具**: 字节操作和转换工具
- **Noncopyable**: RAII和仅移动语义支持
- **变体类型**: 类型安全的联合实现
- **可选类型**: 空安全的可选值容器
- **智能指针**: 增强的指针管理工具

**使用场景**: 泛型编程、类型安全、内存管理

---

### util - 通用工具
**用途**: 通用工具组件和辅助函数

**核心功能**:
- **Defer**: 基于作用域的清理（Go风格defer）
- **一次执行**: 单次执行保证
- **字符串处理**: 增强的字符串操作和解析
- **版本管理**: 版本比较和解析
- **配置**: 设置管理和解析
- **验证**: 数据验证和净化工具

**使用场景**: 应用程序工具、配置管理、数据处理

---

## 集成示例

```cpp
#include "hj/hj.hpp"

// 多模块协同使用
int main() {
    // 日志设置
    hj::log::init("app.log", hj::log::Level::INFO);
    
    // 数据库连接池
    auto db_pool = hj::db::ConnectionPool::create("sqlite:app.db", 10);
    
    // 异步I/O网络服务器
    hj::net::HttpServer server(8080);
    server.route("/api/data", [&](const auto& req, auto& res) {
        // 异步处理请求
        hj::io::async([&]() {
            auto conn = db_pool->get_connection();
            auto result = conn->query("SELECT * FROM users");
            res.json(result.to_json());
        });
    });
    
    // 启动服务器
    server.listen();
    return 0;
}
```

## 性能考虑

- **内存管理**: 所有模块使用高效的内存分配策略
- **线程安全**: 核心模块设计为支持并发访问
- **零拷贝**: 许多操作避免不必要的内存拷贝
- **RAII**: 自动清理的适当资源管理
- **基于模板**: 尽可能的编译时优化

## 兼容性

- **C++17标准**: 完全利用C++17特性
- **跨平台**: 支持Windows、Linux、macOS
- **编译器支持**: GCC 11+、Clang 16+、MSVC 2019+
- **架构**: 支持x86_64、ARM64

---

## 第三方库

本项目集成或兼容下列第三方库（通过 vcpkg 管理或作为可选项）：

- Boost: https://www.boost.org
- nlohmann/json: https://github.com/nlohmann/json
- pugixml: https://github.com/zeux/pugixml
- libzmq (ZeroMQ): https://github.com/zeromq/libzmq
- concurrentqueue: https://github.com/cameron314/concurrentqueue
- spdlog: https://github.com/gabime/spdlog
- googletest: https://github.com/google/googletest
- flatbuffers: https://github.com/google/flatbuffers
- sqlite3: https://github.com/sqlite/sqlite
- BehaviorTree.CPP: https://github.com/BehaviorTree/BehaviorTree.CPP
- cpp-httplib: https://github.com/yhirose/cpp-httplib
- hiredis: https://github.com/redis/hiredis
- hidapi: https://github.com/signal11/hidapi
- breakpad: https://github.com/google/breakpad
- eigen: https://github.com/eigenteam/eigen-git-mirror
- oneTBB: https://github.com/oneapi-src/oneTBB
- fmt: https://github.com/fmtlib/fmt
- pcm: https://github.com/intel/pcm
- benchmark: https://github.com/google/benchmark
- clickhouse-cpp: https://github.com/ClickHouse/clickhouse-cpp
- CUDA Toolkit: https://developer.nvidia.com/cuda-toolkit
- libharu: https://github.com/libharu/libharu
- OpenCL SDK: https://github.com/KhronosGroup/OpenCL-SDK
- zlib: https://github.com/madler/zlib
- yaml-cpp: https://github.com/jbeder/yaml-cpp
- OpenCV: https://github.com/opencv/opencv
- gRPC: https://github.com/grpc/grpc
- hffix: https://jamesdbrock.github.io/hffix
- libpqxx: https://pqxx.org/libpqxx/
- libnice: https://nice.freedesktop.org
- libiconv: https://www.gnu.org/software/libiconv/
- ICU: https://icu.unicode.org/home
- libqrencode: https://github.com/fukuchi/libqrencode
- quirc: https://github.com/dlbeer/quirc