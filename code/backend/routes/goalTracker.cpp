#include <crow.h>
#include <sqlite3.h>
#include "../goalTracker.h"
#include "../helper.h"
#include <vector>
#include <ctime>
#include <iostream>

void setupGoalRoutes(crow::SimpleApp& app, sqlite3* db) {

    // --- GET /goals ---
    CROW_ROUTE(app, "/goals").methods("GET"_method)([db]() {
        auto goals = getAllGoals(db); // now returns full Goal objects

        crow::json::wvalue result;
        std::vector<crow::json::wvalue> arr;

        for (auto& g : goals) {
            g.total_progress = getGoalTotalProgress(db, g.id);
            crow::json::wvalue goal;
            goal["id"] = g.id;
            goal["user_id"] = g.user_id;
            goal["goal_name"] = g.goal_name;
            goal["target_value"] = g.target_value;
            goal["start_date"] = g.start_date;
            goal["end_date"] = g.end_date;
            goal["completed"] = g.completed;
            goal["total_progress"] = g.total_progress; 

            arr.push_back(std::move(goal));
        }

        result["goals"] = std::move(arr);
        return result;
    });

    // --- POST /goals ---
    CROW_ROUTE(app, "/goals").methods("POST"_method)([db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body)
            return makeError(400, "Invalid JSON");

        // Default user_id = 1 if not provided
        int user_id = 1;
        if (body.has("user_id"))
            user_id = static_cast<int>(body["user_id"].i());

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

    // --- PATCH /goals/complete/<goal_id> ---
    CROW_ROUTE(app, "/goals/complete/<int>").methods("PATCH"_method)([db](int goal_id) {
        const char* sql = "UPDATE goals SET completed = 1 WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare completeGoal: " << sqlite3_errmsg(db) << std::endl;
            return makeError(500, "Database error");
        }

        sqlite3_bind_int(stmt, 1, goal_id);
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);

        if (success)
            return makeSuccess(200, "Goal marked complete");
        else
            return makeError(500, "Failed to mark goal complete");
    });
}
