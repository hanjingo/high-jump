# libcpp

cpp library framework implementationed by c++17.

---



## Module

| module   | description                                                  |
| -------- | ------------------------------------------------------------ |
| algo     | -                                                            |
| compress | -                                                            |
| crypto   | -                                                            |
| db       | -                                                            |
| encoding | The module includes functions for processing **hex**, **xml**, and other formats. |
| hardware | -                                                            |
| io       | The module includes functions for processing **tasks** and other related operations. |
| log      | This module encapsulates the **spdlog** project.             |
| math     | This module provides support for **matrix** operations and other related functionalities. |
| misc     | This module includes miscellaneous components, such as **error codes**, **callback hash tables**, and others. |
| net      | This module provides support for **TCP**, **ZeroMQ,** and other related functionalities. |
| os       | -                                                            |
| sync     | This module provides some thread-safe functions and data structures, including **chan**, **once**, **safe list**, and others. |
| testing  | This module encapsulates **Google Test**, **Google breakpad**. |
| time     | This module includes functions for handling **date** and **time** operations. |
| types    | This module includes some commonly used data types, such as **any**, **byte**, **noncopyable**, **variant**, and others. |
| util     | This module includes some commonly used components, such as **defer** and others. |

---



## Build

### Dependencies

| compiler | version         |
| -------- | --------------- |
| GCC      | 11              |
| Clang    | 16              |
| MSVC     | 2019            |

For vcpkg run cmd (reference from: https://learn.microsoft.com/zh-cn/vcpkg/consume/boost-versions):

```cmd
vcpkg install
```

or install 3rd-party project by yourself.

### Build Library

```sh
cmake .. -DBUILD_LIB=ON
```

| option    | default | description                     |
| --------- | ------- | ------------------------------- |
| BUILD_LIB | ON      | build the library if turned on. |

### Build Examples

```sh
cmake .. -DBUILD_EXAMPLE=ON
```

| option        | default | description                             |
| ------------- | ------- | --------------------------------------- |
| BUILD_EXAMPLE | ON      | build the example project if turned on. |

### Build test

```sh
cmake .. -DBUILD_TEST=ON
```

or

```sh
cmake .. -DBUILD_TEST=ON -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake -DASAN=ON
```

| option     | default | description                     |
| ---------- | ------- | ------------------------------- |
| BUILD_TEST | ON      | test this project if turned on. |
| ASAN       | ON      | test this project by ASAN tool. |

---



## License

Please feel free to use this project, as there are no restrictions or legal risks associated with it. If you have any good ideas or suggestions, please contact the author.

---



## 3rd Party

- Boost: https://www.boost.org
- json: https://github.com/nlohmann/json
- pugixml: https://github.com/zeux/pugixml
- libzmq: https://github.com/zeromq/libzmq
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
- hat-trie: https://github.com/Tessil/hat-trie
- pcm: https://github.com/intel/pcm
- benchmark: https://github.com/google/benchmark
- clickhouse-cpp: https://github.com/ClickHouse/clickhouse-cpp
- cuda: https://developer.nvidia.com/cuda-toolkit
- libharu: https://github.com/libharu/libharu
- opencl: https://github.com/KhronosGroup/OpenCL-SDK
- zlib: https://github.com/madler/zlib
- yaml-cpp: https://github.com/jbeder/yaml-cpp
- opencv: https://github.com/opencv/opencv
- grpc: https://github.com/grpc/grpc
- hffix: https://jamesdbrock.github.io/hffix
- libpqxx: https://pqxx.org/libpqxx/
- libnice: https://nice.freedesktop.org
- libiconv: https://www.gnu.org/software/libiconv/
- ICU: https://icu.unicode.org/home