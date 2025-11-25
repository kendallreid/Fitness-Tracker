#ifndef HELPERS_H
#define HELPERS_H

#include <crow.h>
#include <sqlite3.h>
#include <string>
#include <fstream>
#include <sstream>
#include "goalTracker.h"

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

inline crow::json::wvalue serializeGoals(const std::vector<Goal>& goals, sqlite3* db) {
    crow::json::wvalue result;
    std::vector<crow::json::wvalue> arr;
    arr.reserve(goals.size());

    for (const auto& g : goals) {
        crow::json::wvalue goal;
        goal["id"] = g.id;
        goal["user_id"] = g.user_id;
        goal["goal_name"] = g.goal_name;
        goal["target_value"] = g.target_value;
        goal["start_date"] = g.start_date;

        // end_date might be empty string in your struct; that's fine
        goal["end_date"] = g.end_date;
        goal["status"] = g.status;

        // Keep computing total_progress exactly like your handlers do
        goal["total_progress"] = getGoalTotalProgress(db, g.id);

        arr.push_back(std::move(goal));
    }

    result["goals"] = std::move(arr);
    return result;
}
// Date utility functions
std::string getCurrentDate();
std::string getCurrentDateTime();
std::string getDateNDaysAgo(int days);
bool validateMealData(const crow::json::rvalue& data, std::string& error);

#endif // HELPERS_H