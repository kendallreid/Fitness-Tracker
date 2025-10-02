#include <crow.h>
#include <sqlite3.h>
#include <sstream>
#include <map>
#include "../LogIn.h"
#include "../helper.h"

std::string urlDecode(const std::string &s) {
    std::ostringstream out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            std::string hex = s.substr(i + 1, 2);
            char c = static_cast<char>(std::stoi(hex, nullptr, 16));
            out << c;
            i += 2;
        } else if (s[i] == '+') {
            out << ' ';
        } else {
            out << s[i];
        }
    }
    return out.str();
}

std::map<std::string, std::string> parseFormData(const std::string& body) {
    std::map<std::string, std::string> form;
    std::istringstream stream(body);
    std::string pair;

    while (std::getline(stream, pair, '&')) {
        auto pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);

            // decode + and %XX sequences
            value = urlDecode(value);

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
        std::cout << "Raw body: " << req.body << std::endl;
        std::cout << "Parsed username: " << form["username"] << std::endl;
        std::cout << "Parsed password: " << form["password"] << std::endl;

        auto usernameIt = form.find("username");
        auto passwordIt = form.find("password");
    if (usernameIt == form.end() || passwordIt == form.end()) {
            return crow::response(400, "Missing username or password");
        }

        std::string username = usernameIt->second;
        std::string password = passwordIt->second;

        crow::response res;
        if (loginManager.LogIn(username, password, db)) {
            //return crow::response(200, "Login successful!");
            res.code = 302;                          // HTTP redirect
            res.set_header("Location", "/home");
            return res;
        } else {
            res.code = 302;                    
            res.set_header("Location", "/auth/login?error=1");  // Failure - back to login page
            return res;
            //return crow::response(401, "Invalid username or password");
        }
    });
}
