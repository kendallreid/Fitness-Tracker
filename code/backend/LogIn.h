#ifndef LOGIN
#define LOGIN
#include <iostream>
#include <string>

using std::string;

struct User{
    string username;
    string password;  // Store hashed password
};

class LogInManager
{
public:
    LogInManager(const string& dbPath) : _databasePath(dbPath) {};
    bool userExists(const string& username);
    bool getUser(const std::string& username, User& outUser);
    bool LogIn(const string& username, const string& password);

private:
    string _databasePath;
};

#endif