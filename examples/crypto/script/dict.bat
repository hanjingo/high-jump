@echo off
REM Automatically create dict.db and import a user-specified dictionary file (txt/csv)

set MODE=csv
if not "%2"=="" set MODE=%2

REM 1. Create database and table structure
sqlite3 dict.db < dict.sql

REM Check for input file
if "%1"=="" (
    echo Usage: %0 your_dict.txt csv^|txt
    echo Skipping import.
    exit /b 0
)

if not exist "%1" (
    echo File not found: %1
    echo Skipping import.
    exit /b 0
)

REM 2. Import dictionary (assume %1 has one password per line, no header)
sqlite3 dict.db ".mode %MODE%" ".import %1 passwords"

echo Import completed.
