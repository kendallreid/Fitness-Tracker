#include <crow.h>
#include <sqlite3.h>
#include "../goalTracker.h"
#include "../helper.h"
#include <vector>
#include <ctime>
#include <iostream>
#include "helper.h"

void setupGoalRoutes(crow::SimpleApp& app, sqlite3* db) {

    // --- GET /goals/active ---
    CROW_ROUTE(app, "/goals/active").methods("GET"_method)([db](const crow::request& req) {
        auto cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return crow::response{401, "Unauthorized: missing login cookie"};
        }
        int user_id = std::stoi(user_id_str);
        auto goals = getAllGoals(db, user_id, "active"); // now returns full Goal objects

        crow::json::wvalue result;
        std::vector<crow::json::wvalue> arr;

        return crow::response(serializeGoals(goals, db));
    });

    // --- Get /goals/completed ---
    CROW_ROUTE(app, "/goals/completed").methods("GET"_method)([db](const crow::request& req) {
        auto cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return crow::response{401, "Unauthorized: missing login cookie"};
        }

        int user_id = std::stoi(user_id_str);
        auto goals = getAllGoals(db, user_id, "completed"); // now returns full Goal objects

        return crow::response(serializeGoals(goals, db));
    });

    // --- POST /goals ---
    CROW_ROUTE(app, "/goals").methods("POST"_method)([db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body)
            return makeError(400, "Invalid JSON");

        auto cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return crow::response{401, "Unauthorized: missing login cookie"};
        }

        int user_id = std::stoi(user_id_str);
        
        std::string goal_name = "";
        if (body.has("goal_name"))
            goal_name = body["goal_name"].s();

        double target_value = 0.0;
        if (body.has("target_value"))
            target_value = static_cast<double>(body["target_value"].d());

        std::string start_date = "";
        if (body.has("start_date"))
            start_date = body["start_date"].s();

        std::string end_date = "";
        if (body.has("end_date"))
            end_date = body["end_date"].s();

        if (goal_name.empty() || start_date.empty() || target_value <= 0)
            return makeError(400, "Missing required fields");

        bool success = addGoal(db, user_id, goal_name, target_value, start_date, end_date);

        if (success)
            return makeSuccess(201, "Goal added successfully");
        else
            return makeError(500, "Database error");
    });


    // --- POST /goal-progress ---
    CROW_ROUTE(app, "/goal-progress").methods("POST"_method)([db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body)
            return makeError(400, "Invalid JSON");

        int goal_id = body.has("goal_id") ? body["goal_id"].i() : 0;
        double progress_value = body.has("progress_value") ? body["progress_value"].d() : -1;

        if (goal_id <= 0 || progress_value < 0)
            return makeError(400, "Invalid goal or progress value");

        bool success = addGoalProgress(db, goal_id, progress_value);

        if (success)
            return makeSuccess(201, "Progress added successfully");
        else
            return makeError(500, "Database error");
    });

    // PATCH /goals/toggle-complete/<goal_id>
    CROW_ROUTE(app, "/goals/toggle-complete/<int>").methods("PATCH"_method)([db](int goal_id) {
        // Get current status
        std::string status;
        const char* selectSql = "SELECT status FROM goals WHERE id = ?;";
        sqlite3_stmt* selectStmt;
        if (sqlite3_prepare_v2(db, selectSql, -1, &selectStmt, nullptr) != SQLITE_OK)
            return makeError(500, "Database error");

        sqlite3_bind_int(selectStmt, 1, goal_id);
        if (sqlite3_step(selectStmt) == SQLITE_ROW)
            status = reinterpret_cast<const char*>(sqlite3_column_text(selectStmt, 0));
        sqlite3_finalize(selectStmt);

        // Toggle
        std::string newState = (status == "active") ? "completed" : "active";

        const char* updateSql = "UPDATE goals SET status = ? WHERE id = ?;";
        sqlite3_stmt* updateStmt;
        if (sqlite3_prepare_v2(db, updateSql, -1, &updateStmt, nullptr) != SQLITE_OK)
            return makeError(500, "Database error");

        sqlite3_bind_text(updateStmt, 1, newState.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(updateStmt, 2, goal_id);
        bool success = (sqlite3_step(updateStmt) == SQLITE_DONE);
        sqlite3_finalize(updateStmt);

        if (success)
            return makeSuccess(200, newState == "completed" ? "Goal marked complete" : "Goal marked incomplete");
        else
            return makeError(500, "Failed to toggle goal complete");
    });

    CROW_ROUTE(app, "/goals/<int>").methods("DELETE"_method)([db](int goal_id) {
        const char* sql = "DELETE FROM goals WHERE id = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {

            std::cerr << sqlite3_errmsg(db) << std::endl;
            return makeError(500, "Database error");
        }
        sqlite3_bind_int(stmt, 1, goal_id);
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);

        if (success){
            return makeSuccess(200, "Goal deleted successfully");
        } else {
            std::cerr << sqlite3_errmsg(db) << std::endl;
            return makeError(500, "Failed to delete goal");
        }
    });

    CROW_ROUTE(app, "/goals/<int>/complete").methods("POST"_method)([db](int goal_id) {
        const char* sql = "UPDATE goals SET status = 'completed', updated_at = datetime('now') WHERE id = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, goal_id);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc == SQLITE_DONE)
            return makeSuccess(200, "Goal marked as completed");
        else
            return makeError(500, sqlite3_errmsg(db));
    });


}
