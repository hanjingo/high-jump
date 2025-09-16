CREATE TABLE IF NOT EXISTS passwords (
    password TEXT PRIMARY KEY COLLATE BINARY
) WITHOUT ROWID;

CREATE TABLE IF NOT EXISTS password_counts (
    password TEXT PRIMARY KEY COLLATE BINARY,
    hit_count INTEGER NOT NULL DEFAULT 0
) WITHOUT ROWID;

PRAGMA synchronous = OFF;
PRAGMA journal_mode = OFF;
PRAGMA temp_store = MEMORY;
PRAGMA cache_size = -200000; -- 200MB
PRAGMA locking_mode = EXCLUSIVE;

-- sqlite3 dict.db
-- > .mode line
-- > .import --skip 1 --insert "INSERT OR IGNORE INTO passwords VALUES(?)" rockyou.txt