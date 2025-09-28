@echo off
REM Windows batch script to run unit tests and output coverage
REM Requires OpenCppCoverage.exe in PATH

setlocal
set BUILD_DIR=../build
set BIN_DEBUG=../bin/Debug
set BIN_RELEASE=../bin/Release
set TEST_EXE=

if exist %BIN_DEBUG%\tests.exe (
    set TEST_EXE=%BIN_DEBUG%\tests.exe
) else if exist %BIN_RELEASE%\tests.exe (
    set TEST_EXE=%BIN_RELEASE%\tests.exe
) else (
    echo tests.exe not found in Debug or Release bin directory.
    exit /b 1
)

set COVERAGE_EXE=OpenCppCoverage.exe

REM Build with coverage flags
echo Building with coverage...
cmake -S .. -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="/MP /Zi /Od /RTC1 /DTEST /DDEBUG /DWIN32 /D_WINDOWS /EHsc /MDd /FC /W3 /WX-"
for /f "tokens=2 delims==" %%C in ('wmic cpu get NumberOfLogicalProcessors /value') do set CORES=%%C
if "%CORES%"=="" set CORES=1
set /a THREADS=%CORES%-1
if %THREADS% lss 1 set THREADS=1
cmake --build %BUILD_DIR% -- /m:%THREADS%

if not exist %COVERAGE_EXE% (
    echo OpenCppCoverage.exe not found. Please install OpenCppCoverage and add it to PATH.
    exit /b 1
)

echo Running tests with coverage...
%COVERAGE_EXE% --sources %BUILD_DIR% --export_type html --export_type cobertura -- %TEST_EXE%
endlocal
