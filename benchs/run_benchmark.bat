@echo off
setlocal enabledelayedexpansion

REM Benchmark runner script with timestamped results export and optional comparison for Windows
REM Usage: run_benchmark.bat [/compare] [benchmark_options]

REM Parse command line arguments
set "ENABLE_COMPARE=false"
set "BENCHMARK_ARGS="

:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="/compare" (
    set "ENABLE_COMPARE=true"
    shift
    goto :parse_args
)
set "BENCHMARK_ARGS=!BENCHMARK_ARGS! %~1"
shift
goto :parse_args
:done_parsing

REM Get the directory of this script
set "SCRIPT_DIR=%~dp0"
set "RESULTS_DIR=%SCRIPT_DIR%results"
set "BIN_DIR=%SCRIPT_DIR%..\bin"

REM Create results directory if it doesn't exist
if not exist "%RESULTS_DIR%" mkdir "%RESULTS_DIR%"

REM Generate timestamp
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"
set "TIMESTAMP=%YYYY%%MM%%DD%_%HH%%Min%%Sec%"

set "JSON_FILE=%RESULTS_DIR%\benchmark_%TIMESTAMP%.json"
set "TXT_FILE=%RESULTS_DIR%\benchmark_%TIMESTAMP%.txt"

REM Check if benchmark binary exists
set "BENCHMARK_BIN=%BIN_DIR%\benchs.exe"
if not exist "%BENCHMARK_BIN%" (
    echo Error: Benchmark binary not found at %BENCHMARK_BIN%
    echo Please build the project first with: cmake --build . --target benchs
    exit /b 1
)

echo Running benchmarks...
echo Timestamp: %TIMESTAMP%
echo Results will be saved to:
echo   JSON: %JSON_FILE%
echo   Text: %TXT_FILE%
echo.

REM Run benchmark with both console output and JSON export
REM Console output is also saved to text file for reference
"%BENCHMARK_BIN%" --benchmark_min_time=0.1 --benchmark_format=json --benchmark_out="%JSON_FILE%" !BENCHMARK_ARGS! > "%TXT_FILE%" 2>&1

if %errorlevel% equ 0 (
    echo.
    echo ✓ Benchmark completed successfully!
    echo Results saved to:
    echo   JSON: %JSON_FILE%
    echo   Text: %TXT_FILE%
    
    REM Show file sizes for reference
    echo.
    echo File sizes:
    for %%f in ("%JSON_FILE%") do echo   %%~nxf: %%~zf bytes
    for %%f in ("%TXT_FILE%") do echo   %%~nxf: %%~zf bytes
    
    REM Run comparison if requested
    if /i "!ENABLE_COMPARE!"=="true" (
        echo.
        echo Running comparison with previous results...
        
        REM Check if generic tool exists
        set "GENERIC_TOOL=%SCRIPT_DIR%..\scripts\benchmark.py"
        if exist "!GENERIC_TOOL!" (
            python "!GENERIC_TOOL!" compare ^
                --results-dir=results ^
                --file-pattern="benchmark_(\\d{8}_\\d{6})\\.json" ^
                --timestamp-format="%%Y%%m%%d_%%H%%M%%S" ^
                --name-prefix="BM_" ^
                --threshold=1.0 ^
                --run-command="run_benchmark.bat"
        ) else (
            echo Warning: Generic benchmark tool not found at !GENERIC_TOOL!
        )
    ) else (
        echo.
        echo To compare with previous results, run:
        echo   run_benchmark.bat /compare
        echo.
        echo Or manually compare with:
        echo   python ..\scripts\benchmark.py compare ^
        echo     --results-dir=results ^
        echo     --file-pattern="benchmark_(\\d{8}_\\d{6})\\.json" ^
        echo     --timestamp-format="%%Y%%m%%d_%%H%%M%%S" ^
        echo     --name-prefix="BM_" ^
        echo     --threshold=1.0 ^
        echo     --run-command="run_benchmark.bat"
    )
) else (
    echo.
    echo ✗ Benchmark failed!
    exit /b 1
)

endlocal