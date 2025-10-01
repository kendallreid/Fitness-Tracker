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
    LogInManager(string& dbPath);
    bool LogIn(const string& username, const string& password);
    bool userExists(const string& username);
    bool getUser(const std::string& username, User& outUser);

private:
    string databasePath;
};

#endif