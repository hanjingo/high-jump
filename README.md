# high-jump (hj)

A modern, modular C++17 utility library focused on performance, portability and developer ergonomics.

Quick, practical building blocks for networking, crypto, IO, algorithms and system utilities — designed for production systems and developer productivity.

---

Highlights
- Lightweight, header-first-friendly APIs where appropriate.
- Battery-included tooling: tests, benchmarks and CI-friendly CMake setup.
- Portable to Linux/macOS/Windows using vcpkg and CMake.
- Focus on safety (RAII), clear error handling (std::error_code) and test coverage.

---

Quick start
1. Install vcpkg and required toolchain. See `vcpkg.json` for dependencies.

Windows (PowerShell):

```powershell
# from project root
cmake -S . -B build -G "Visual Studio 16 2019" -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DBUILD_TEST=ON -DBUILD_BENCH=ON
cmake --build build --config Debug
cmake --build build --target tests --config Debug
```

macOS / Linux:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DBUILD_TEST=ON -DBUILD_BENCH=ON
cmake --build build
ctest --test-dir build
```

Modules (selected)
- algo — data structures & algorithms (skiplist, bloom filters, behavior trees)
- crypto — AES/DES/RSA/MD5/SHA, base64 utilities
- io — file/buffer utilities and async helpers
- net — TCP/UDP/HTTP/ZeroMQ helpers
- os — platform compatibility, process, env, signal
- sync — concurrency primitives and coroutine helpers
- testing — testing helpers and crash/debug utilities
- more detailed module docs: `docs/modules.md` and `docs/modules_zh.md`

Benchmarks & Examples
- Examples: `examples/` contains various usage examples.
- Benchmarks: `benchs/` contains micro-benchmarks using Google Benchmark.

Contributing
- Please open issues or pull requests. Follow the existing code style and add tests for new behavior.

License
- GPL-3.0 — see `LICENSE` for details.
