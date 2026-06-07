# log_core

Windows (PowerShell):

```powershell
# from project root
cmake -S . -B build -G "Visual Studio 16 2019" -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DBUILD_TEST=ON -DBUILD_BENCH=ON
cmake --build build --config Debug
```

macOS / Linux:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DBUILD_TEST=ON -DBUILD_BENCH=ON
cmake --build build
```