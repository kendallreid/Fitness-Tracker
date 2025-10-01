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

#endif // HELPERS_H