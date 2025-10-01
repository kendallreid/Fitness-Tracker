#include <crow.h>
#include <sqlite3.h>
#include <sstream>
#include <map>
#include "../LogIn.h"
#include "../helper.h"

std::map<string, string> parseFormData(const string& body) {
    std::map<string, string> form;
    std::istringstream stream(body);
    string pair;

    while (std::getline(stream, pair, '&')) {
        auto pos = pair.find('=');
        if (pos != string::npos) {
            string key = pair.substr(0, pos);
            string value = pair.substr(pos + 1);

            // decode + and %20 → space
            for (auto &c : value) {
                if (c == '+') c = ' ';
            }
            form[key] = value;
        }
    }
    return form;
}

void setupLoginRoutes(crow::SimpleApp& app, LogInManager& loginManager, sqlite3* db) 
{
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)([&loginManager, db](const crow::request& req)
    {
        auto form = parseFormData(req.body);

        auto usernameIt = form.find("username");
        auto passwordIt = form.find("password");
    if (usernameIt == form.end() || passwordIt == form.end()) {
            return crow::response(400, "Missing username or password");
        }

        std::string username = usernameIt->second;
        std::string password = passwordIt->second;

        if (loginManager.LogIn(username, password, db)) {
            return crow::response(200, "Login successful!");
        } else {
            return crow::response(401, "Invalid username or password");
        }
    });
}