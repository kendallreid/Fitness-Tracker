#include "sleep_tracker.h"
#include "../helper.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

void setupSleepTrackerRoutes(crow::SimpleApp& app, sqlite3* db) {
    CROW_ROUTE(app, "/sleep-tracker")
    ([] {
        return serveFile("code/frontend/SleepTracker.html", "text/html");
    });

    CROW_ROUTE(app, "/api/sleeps").methods("POST"_method)
    ([&app, db](const crow::request& req) {
    return addSleep(app, db, req);
    });

    CROW_ROUTE(app, "/api/sleeps/<int>/<string>").methods("GET"_method)
    ([&app, db](const crow::request& req, int user_id, const std::string& date) {
    return getSleeps(app, db, user_id, date);
    });

    CROW_ROUTE(app, "/api/sleeps/<int>").methods("PUT"_method)
    ([&app, db](const crow::request& req, int sleep_id) {
        return updateSleep(app, db, sleep_id, req);
    });

    CROW_ROUTE(app, "/api/sleeps/<int>").methods("DELETE"_method)
    ([&app, db](int sleep_id) {
       return deleteSleep(app, db, sleep_id);
    });

    CROW_ROUTE(app, "/api/sleeps/clear/<int>/<string>").methods("DELETE"_method)
    ([&app, db](int user_id, const std::string& date) {
       return clearWeeklySleeps(app, db, user_id, date);
    });

    /*CROW_ROUTE(app, "/api/goals/<int>").methods("GET"_method)
    ([&app, db](int user_id) {
        return getUserGoals(app, db, user_id);
    });

    CROW_ROUTE(app, "/api/goals/<int>").methods("PUT"_method)
    ([&app, db](const crow::request& req, int user_id) {
        return updateUserGoals(app, db, user_id, req);
    });*/
}


crow::response addSleep(crow::SimpleApp& app, sqlite3* db, const crow::request& req) {
    auto data = crow::json::load(req.body);
    if (!data) return crow::response(400, "Invalid JSON");

    std::string error;
    if (!validateSleepData(data, error)) return crow::response(400, error);

    int user_id = data["user_id"].i();
    //int sleep_id = data.has("id") ? data["id"].s() : getCurrentDate();
    std::string date = data["date"].s();
    std::string time = data["time"].s();
    int duration = data["duration"].i();
    std::string sleep_type = data["sleep_type"].s();
    std::string created_at = getCurrentDateTime();

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "INSERT INTO sleep (user_id, date, time, duration, sleep_quality, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)",
        -1, &stmt, nullptr);
    
    sqlite3_bind_int(stmt, 1, user_id);
    //sqlite3_bind_int(stmt, 2, sleep_id);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, time.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, duration);
    sqlite3_bind_text(stmt, 5, sleep_type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, created_at.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return crow::response(500, "Database insert failed");
    }

    int sleep_id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    return crow::response(201, "{\"sleep_id\":" + std::to_string(sleep_id) + "}");
}


crow::response getSleeps(crow::SimpleApp& app, sqlite3* db, int user_id, const std::string& date) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "SELECT sleep_id, date, time, duration, sleep_quality, created_at "
        "FROM sleep WHERE user_id=? AND date<=? AND date>=? ORDER BY created_at DESC",
        -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_STATIC);

    crow::json::wvalue sleeps;
    std::vector<crow::json::wvalue> sleep_list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        crow::json::wvalue sleep;
        sleep["id"] = sqlite3_column_int(stmt, 0);
        sleep["date"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        sleep["time"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        sleep["duration"] = sqlite3_column_int(stmt, 3);
        sleep["sleep_type"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        sleep["created_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        sleep_list.push_back(std::move(sleep));
    }

    sqlite3_finalize(stmt);
    return crow::response(sleeps);
}

crow::response deleteSleep(crow::SimpleApp&, sqlite3* db, int sleep_id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "DELETE FROM sleep WHERE user_id=? AND sleep_id=?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, sleep_id);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    return ok ? crow::response(200, "Deleted") : crow::response(500, "Failed");
}

crow::response updateSleep(crow::SimpleApp& app, sqlite3* db, int sleep_id, const crow::request& req) {
    auto data = crow::json::load(req.body);
    if (!data) return crow::response(400, "Invalid JSON");

    std::string error;
    if (!validateSleepData(data, error)) return crow::response(400, error);

    std::string date = data["date"].s();
    std::string time = data["time"].s();
    int duration = data["duration"].i();
    std::string sleep_type = data["sleep_type"].s();

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "UPDATE sleep SET date=?, time=?, duration=?, sleep_type=? WHERE sleep_id=?",
        -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, time.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, duration);
    sqlite3_bind_text(stmt, 4, sleep_type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, sleep_id);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    return ok ? crow::response(200, "Updated") : crow::response(500, "Update failed");
}


crow::response clearWeeklySleeps(crow::SimpleApp& app, sqlite3* db, int user_id, const std::string& date) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "DELETE FROM sleep WHERE user_id=? AND date<=? AND date>=?",
        -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_STATIC);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    return ok ? crow::response(200, "Cleared") : crow::response(500, "Failed to clear sleeps");
}
    
bool validateSleepData(const crow::json::rvalue &data, std::string &error)
{
    return false;
}

