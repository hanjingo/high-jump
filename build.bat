@echo off
REM Windows batch script for Release build: examples and library only
REM Usage: build.bat

setlocal
set BUILD_DIR=build

REM Check VCPKG_ROOT environment variable
if "%VCPKG_ROOT%"=="" (
	echo Error: VCPKG_ROOT environment variable is not set. Please set it to your vcpkg root directory.
	exit /b 1
)

REM Detect Visual Studio version
set VS_GENERATOR=""
where /q devenv.exe
if %ERRORLEVEL%==0 (
	REM Try VS2019
	reg query "HKLM\SOFTWARE\Microsoft\VisualStudio\SxS\VS7" /v 16.0 >nul 2>nul
	if %ERRORLEVEL%==0 (
		set VS_GENERATOR=Visual Studio 16 2019
	) else (
		REM Try VS2022
		reg query "HKLM\SOFTWARE\Microsoft\VisualStudio\SxS\VS7" /v 17.0 >nul 2>nul
		if %ERRORLEVEL%==0 (
			set VS_GENERATOR=Visual Studio 17 2022
		) else (
			echo Error: No supported Visual Studio version (2019/2022) found!
			exit /b 1
		)
	)
) else (
	echo Error: Visual Studio (devenv.exe) not found in PATH!
	exit /b 1
)

if not exist %BUILD_DIR% mkdir %BUILD_DIR%

REM Configure for Release, build examples and library
cmake -S . -B %BUILD_DIR% -G "%VS_GENERATOR%" -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLE=ON -DBUILD_LIB=ON -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"

REM Detect CPU cores for parallel build
for /f "tokens=2 delims==" %%C in ('wmic cpu get NumberOfLogicalProcessors /value') do set CORES=%%C
if "%CORES%"=="" set CORES=1
set /a THREADS=%CORES%-1
if %THREADS% lss 1 set THREADS=1

cmake --build %BUILD_DIR% --config Release -- /m:%THREADS%

endlocal
endlocal
