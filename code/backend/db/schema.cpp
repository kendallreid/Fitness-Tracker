#include "schema.h"
#include <iostream>
using namespace std;

bool createTables(sqlite3 *db)
{
    const char *sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            first_name TEXT NOT NULL,
            last_name TEXT NOT NULL,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            email TEXT UNIQUE NOT NULL,
            score INTEGER DEFAULT 0
        );

        CREATE TABLE IF NOT EXISTS sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            date TEXT NOT NULL,
            notes TEXT,
            duration INTEGER,
            created_at TEXT DEFAULT (datetime('now')),
            updated_at TEXT DEFAULT (datetime('now')),
            FOREIGN KEY (user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS exercises (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            date TEXT NOT NULL,
            type TEXT NOT NULL,
            sets INTEGER,
            reps INTEGER,
            weight REAL,
            duration INTEGER,
            session_id INTEGER,
            notes TEXT,
            FOREIGN KEY(session_id) REFERENCES sessions(id),
            FOREIGN KEY(user_id) REFERENCES users(id)
        );
 
        CREATE TABLE IF NOT EXISTS nutrition (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            date TEXT NOT NULL,
            meal_type TEXT NOT NULL,
            meal_name TEXT NOT NULL,
            calories INTEGER NOT NULL,
            protein REAL DEFAULT 0,
            created_at TEXT NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS sleepTable (
            sleep_id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            sleep_start_time TIMESTAMP NOT NULL,
            duration INTEGER NOT NULL,
            sleep_type TEXT NOT NULL,
            created_at TEXT NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS user_goals (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            daily_calorie_goal INTEGER DEFAULT 2000,
            daily_protein_goal REAL DEFAULT 150.0,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS goals (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            goal_name TEXT NOT NULL,
            target_value REAL,
            current_value REAL DEFAULT 0,
            status TEXT DEFAULT 'active',
            frequency TEXT DEFAULT 'none',
            start_date TEXT DEFAULT (datetime('now')),
            end_date TEXT,
            updated_at TEXT DEFAULT (datetime('now')),
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS goal_progress (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            goal_id INTEGER NOT NULL,
            date TEXT NOT NULL,
            progress_value REAL NOT NULL,
            FOREIGN KEY(goal_id) REFERENCES goals(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS friend_requests (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender_id   INTEGER NOT NULL,
            receiver_id INTEGER NOT NULL,
            status      TEXT NOT NULL CHECK (status IN ('pending', 'accepted', 'rejected', 'cancelled')),
            created_at  TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,

            -- Don't allow someone to send a request to themselves
            CHECK (sender_id <> receiver_id),

            -- Only one active record per ordered pair
            UNIQUE (sender_id, receiver_id),

            FOREIGN KEY (sender_id)   REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (receiver_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_friend_requests_receiver_status
            ON friend_requests (receiver_id, status);

        CREATE INDEX IF NOT EXISTS idx_friend_requests_sender_status
            ON friend_requests (sender_id, status);

        CREATE TABLE IF NOT EXISTS friendships (
            user_id1   INTEGER NOT NULL,
            user_id2   INTEGER NOT NULL,
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,

            -- Enforce symmetric representation: (min, max)
            CHECK (user_id1 < user_id2),

            PRIMARY KEY (user_id1, user_id2),

            FOREIGN KEY (user_id1) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (user_id2) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS password_reset_tokens (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            token TEXT NOT NULL UNIQUE,      -- random, hard-to-guess string
            expires_at TEXT NOT NULL,        -- ISO 8601 UTC, e.g. "2025-11-26T18:30:00Z"
            used INTEGER NOT NULL DEFAULT 0, -- 0 = not used, 1 = used
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_password_reset_user_id
            ON password_reset_tokens(user_id);
)";

    // sqlite3_exec executes the queries that are provided to it in the message above. In this case, it will create the tables if they do not already exist.
    // sqlite3_free is used to free the memory allocated for the error message if an error occurs. Otherwise it will leak memory.

    char *errMsg = nullptr;
    int status = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (status != SQLITE_OK) {
        std::cerr << "Error creating tables: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}