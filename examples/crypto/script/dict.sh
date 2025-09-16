#!/bin/bash
# Automatically create dict.db and import a user-specified dictionary file (txt/csv)

MODE="csv"
if [ $# -ge 2 ]; then
    MODE="$2"
fi

# 1. Create database and table structure
sqlite3 dict.db < dict.sql

# Check for input file
if [ $# -lt 1 ]; then
    echo "Usage: $0 your_dict.txt csv|txt"
    echo "Skipping import."
    exit 0
fi

if [ ! -f "$1" ]; then
    echo "File not found: $1"
    echo "Skipping import."
    exit 0
fi

# 2. Import dictionary (assume $1 has one password per line, no header)
sqlite3 dict.db ".mode $MODE" ".import $1 passwords"

echo "Import completed."