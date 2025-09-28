#include "schema.h"

bool create_tables(sqlite3 *db)
{
    const char *sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS workouts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            date TEXT NOT NULL,
            type TEXT NOT NULL,
            duration INTEGER NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS nutrition (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            date TEXT NOT NULL,
            meal_type TEXT NOT NULL,
            calories INTEGER NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id)
        );
    )";

    // sqlite3_exec executes the queries that are provided to it in the message above. In this case, it will create the tables if they do not already exist.
    // sqlite3_free is used to free the memory allocated for the error message if an error occurs. Otherwise it will leak memory.

    char *errMsg = nullptr;
    int status = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (status != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}