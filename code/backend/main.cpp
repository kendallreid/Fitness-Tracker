#include <crow.h>
#include <sodium.h>
#include "routes/register.h"
#include "db/schema.h"
#include <iostream>
using namespace std;

// helpers to create JSON responses
inline crow::response makeError(int code, const string& message) {
    crow::json::wvalue body;
    body["status"] = "error";
    body["message"] = message;
    return crow::response(code, body);
}

inline crow::response makeSuccess(int code, const string& message) {
    crow::json::wvalue body;
    body["status"] = "success";
    body["message"] = message;
    return crow::response(code, body);
}

crow::response serveFile(const string& filepath, const string& contentType) {
    // open file and read content
    ifstream file(filepath, ios::in | ios::binary);
    if (!file) {
        return makeError(404, "File not found");
    }

    // read file content into a string
    ostringstream buffer;
    buffer << file.rdbuf();

    // create response
    crow::response res(buffer.str());
    res.set_header("Content-Type", contentType);
    return res;
}

int main() {

    // Initialize SQLite database connection
    sqlite3* db;
    if (sqlite3_open("code/backend/fitness.db", &db)) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    if (!createTables(db)) {
    cerr << "Failed to create tables" << endl;
    sqlite3_close(db);
    return 1;
    }

    crow::SimpleApp fitnessApp;

    // Initialize libsodium
    if (sodium_init() < 0) {
        std::cerr << "Failed to initialize libsodium" << std::endl;
        return 1; // don’t continue if init fails
    }

    // Serve registration page
    CROW_ROUTE(fitnessApp, "/auth/register")
    ([]{
        return serveFile("code/frontend/UserRegistration.html", "text/html");
    });

    // User registration route
    CROW_ROUTE(fitnessApp, "/register")
    .methods("POST"_method)([db](const crow::request& req){

        // Parse JSON body
        auto body = crow::json::load(req.body);
        if (!body) {
            return makeError(400, "Invalid JSON");
        }

        string username, email, password, firstName, lastName;

        username = body["username"].s();
        email = body["email"].s();
        password = body["password"].s();
        firstName = body["firstName"].s();
        lastName = body["lastName"].s();

        // Basic validation
        if (username.empty() || email.empty() || password.empty() || firstName.empty() || lastName.empty()) {
            return makeError(400, "Missing required fields");
        }

        // Create user
        CreateUserResult result = createUser(db, username, password, email, firstName, lastName);

        // Handle result
        switch (result) {
            case CreateUserResult::Success:
                return makeSuccess(201, "User created successfully");
            case CreateUserResult::EmailAlreadyExists:
                return makeError(409, "Email already exists");
            case CreateUserResult::UsernameAlreadyExists:
                return makeError(409, "Username already exists");
            case CreateUserResult::DatabaseError:
            default:
                return makeError(500, "Database error");
        }

    });

    fitnessApp.port(8080).multithreaded().run();
    sqlite3_close(db);

    return 0;
}
