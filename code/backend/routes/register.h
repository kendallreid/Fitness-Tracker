#pragma once
#include <string>
#include <sodium.h>
#include <stdexcept>
#include <sqlite3.h>
#include <iostream>
#include "hash.h"
#include "../helper.h"

using namespace std;

enum class CreateUserResult {
    Success,
    EmailAlreadyExists,
    UsernameAlreadyExists,
    DatabaseError
};

struct User {
    int id;
    string username;
    string email;
    string passwordHash;
    string firstName;
    string lastName;
};

struct UserResult {
    CreateUserResult result;
    User user; // valid only if result == Success
};

CreateUserResult createUser(sqlite3* db, const string& username, const string& password, const string& email, const string& firstName, const string& lastName);
int insertUserIntoDB(sqlite3* db, const User& user); // Placeholder for actual DB insertion function
void setupRegisterRoutes(crow::SimpleApp& app, sqlite3* db);
