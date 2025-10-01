#include "register.h"

CreateUserResult createUser(sqlite3* db, const string &username, const string &password, const string &email, const string &firstName, const string &lastName)
{
    User user;

    user.passwordHash = hashPassword(password);
    user.username = username;
    user.email = email;
    user.firstName = firstName;
    user.lastName = lastName;

    // returns SQLITE_DONE on success, SQLITE_CONSTRAINT if email or username already exists, or other error codes for different failures
    int rc = insertUserIntoDB(db, user);

    if (rc == SQLITE_DONE)
    {
        // Get the last inserted row ID
        user.id = static_cast<int>(sqlite3_last_insert_rowid(db));
        return CreateUserResult::Success;
    }

    if (rc == SQLITE_CONSTRAINT)
    {
        // Check which constraint was violated
        string errMsg = sqlite3_errmsg(db);
        if (errMsg.find("users.email") != string::npos) // check if the error message contains "users.email"
        {
            return CreateUserResult::EmailAlreadyExists;
        }
        else if (errMsg.find("users.username") != string::npos) // check if the error message contains "users.username"
        {
            return CreateUserResult::UsernameAlreadyExists;
        }
    }  

    // For any other error
    return CreateUserResult::DatabaseError;
}

int insertUserIntoDB(sqlite3 *db, const User &user)
{
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO users (username, email, password_hash, first_name, last_name) VALUES (?, ?, ?, ?, ?);";

    // creates the statement object
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        return rc; // Error
    }

    // binds the parameters to the statement
    sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.firstName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.lastName.c_str(), -1, SQLITE_TRANSIENT);

    // executes the statement, returns SQLITE_DONE on success
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) 
    {
        sqlite3_finalize(stmt);
        cerr << "SQLite insert error: " << sqlite3_errmsg(db) << endl;
        return rc; // Error
    }

    // finalizes the statement to release resources
    sqlite3_finalize(stmt);
    return rc; // Success
}

