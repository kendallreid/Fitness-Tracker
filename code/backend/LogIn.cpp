#include "LogIn.h"
#include "hash.h"
#include <sqlite3.h>
#include <iostream>

using std::cerr;
using std::endl;

bool LogInManager::userExists(const string& username, sqlite3* db)
{
    User user;  // Dummy value for getUser argument
    return getUser(username, user, db);
}

bool LogInManager::getUser(const std::string& username, User& outUser, sqlite3* db)
{
    // Open the database
    //sqlite3* db;
    //if (sqlite3_open(_databasePath.c_str(), &db))
    //{
    //    cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
    //    return false;
    //}

    // Create a query
    string query = "SELECT username, password_hash FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;
    if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    // Bind actual username string to ?
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    // Executes query
    bool found = false;
    if(sqlite3_step(stmt) == SQLITE_ROW)
    {
        outUser.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        outUser.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        found = true;
    }

    // Clear space created from query and close database
    sqlite3_finalize(stmt);
    //sqlite3_close(db);
    return found;
}

bool LogInManager::LogIn(const string& username, const string& password, sqlite3* db)
{
    User user;

    if (!getUser(username, user, db))  // User not found from fetch
        return false;
    
    if (verifyPassword(password, user.password))  // NEED TO HASH THIS EVENTUALLY
        return true;  // Login successful
    else
        return false;  // Wrong password
}