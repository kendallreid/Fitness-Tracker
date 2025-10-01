#include <string>
#include <sodium.h>
#include <stdexcept>
#include <sqlite3.h>
#include <iostream>

using namespace std;

string hashPassword(const string& password);
bool verifyPassword(const string &password, const string &hashed);