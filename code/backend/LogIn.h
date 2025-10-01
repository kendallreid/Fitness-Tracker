#ifndef LOGIN
#define LOGIN
#include "routes/register.h"
#include <iostream>
#include <sqlite3.h>
#include <string>

using std::string;

//struct User{
  //  string username;
    //string password;  // Store hashed password
//};

class LogInManager
{
public:
    LogInManager() {};
    //LogInManager(const string& dbPath) : _databasePath(dbPath) {};
    bool userExists(const string& username, sqlite3* db);
    bool getUser(const std::string& username, User& outUser, sqlite3* db);
    bool LogIn(const string& username, const string& password, sqlite3* db);

private:
    //string _databasePath;
};

void setupLoginRoutes(crow::SimpleApp& app, LogInManager& loginManager, sqlite3* db);

#endif