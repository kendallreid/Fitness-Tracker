#ifndef HELPERS_H
#define HELPERS_H

#include <crow.h>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

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

inline crow::response serveFile(const string& filepath, const string& contentType) {
    ifstream file(filepath, ios::in | ios::binary);
    if (!file) {
        return makeError(404, "File not found");
    }

    ostringstream buffer;
    buffer << file.rdbuf();

    crow::response res(buffer.str());
    res.set_header("Content-Type", contentType);
    return res;
}

inline std::string getCookieValue(const std::string& cookieHeader, const std::string& key)
{
    size_t start = cookieHeader.find(key + "=");
    if (start == std::string::npos) return "";

    start += key.size() + 1;
    size_t end = cookieHeader.find(";", start);
    return cookieHeader.substr(start, end - start);
}

#endif // HELPERS_H