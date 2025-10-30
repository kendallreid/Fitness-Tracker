#include "calorie_tracker.h"
#include "../helper.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

void setupCalorieTrackerRoutes(crow::SimpleApp& app, sqlite3* db) {
    // Serve the calorie tracker page
    CROW_ROUTE(app, "/calorie-tracker")
    ([] {
        return serveFile("code/frontend/CalorieTracker.html", "text/html");
    });

    // Add meal
    CROW_ROUTE(app, "/api/meals").methods("POST"_method)
    ([&app, db](const crow::request& req, crow::response& res) {
        res = addMeal(app, db, req);
        res.end();
    });

    // Get meals for a specific user + date
    CROW_ROUTE(app, "/api/meals/<int>/<string>").methods("GET"_method)
    ([&app, db](const crow::request& req, crow::response& res, int user_id, const std::string& date) {
        res = getMeals(app, db, user_id, date);
        res.end();
    });

    // Update a meal
    CROW_ROUTE(app, "/api/meals/<int>").methods("PUT"_method)
    ([&app, db](const crow::request& req, crow::response& res, int meal_id) {
        res = updateMeal(app, db, meal_id, req);
        res.end();
    });

    // Delete a meal
    CROW_ROUTE(app, "/api/meals/<int>").methods("DELETE"_method)
    ([&app, db](const crow::request& req, crow::response& res, int meal_id) {
        res = deleteMeal(app, db, meal_id);
        res.end();
    });

    // Clear all meals for a day
    CROW_ROUTE(app, "/api/meals/clear/<int>/<string>").methods("DELETE"_method)
    ([&app, db](const crow::request& req, crow::response& res, int user_id, const std::string& date) {
        res = clearDayMeals(app, db, user_id, date);
        res.end();
    });

    // Get user goals
    CROW_ROUTE(app, "/api/goals/<int>").methods("GET"_method)
    ([&app, db](const crow::request& req, crow::response& res, int user_id) {
        res = getUserGoals(app, db, user_id);
        res.end();
    });

    // Update user goals
    CROW_ROUTE(app, "/api/goals/<int>").methods("PUT"_method)
    ([&app, db](const crow::request& req, crow::response& res, int user_id) {
        res = updateUserGoals(app, db, user_id, req);
        res.end();
    });
}



crow::response addMeal(crow::SimpleApp& app, sqlite3* db, const crow::request& req) {
    auto data = crow::json::load(req.body);
    if (!data) return crow::response(400, "Invalid JSON");

    std::string error;
    if (!validateMealData(data, error)) return crow::response(400, error);

    int user_id = data["user_id"].i();
    std::string date = data.has("date") ? data["date"].s() : getCurrentDate();
    std::string meal_type = data["meal_type"].s();
    std::string meal_name = data["meal_name"].s();
    int calories = data["calories"].i();
    double protein = data.has("protein") ? data["protein"].d() : 0.0;
    std::string created_at = getCurrentDateTime();

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "INSERT INTO nutrition (user_id, date, meal_type, meal_name, calories, protein, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)",
        -1, &stmt, nullptr);
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, meal_type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, meal_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, calories);
    sqlite3_bind_double(stmt, 6, protein);
    sqlite3_bind_text(stmt, 7, created_at.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return crow::response(500, "Database insert failed");
    }

    int meal_id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    return crow::response(201, "{\"meal_id\":" + std::to_string(meal_id) + "}");
}


crow::response getMeals(crow::SimpleApp& app, sqlite3* db, int user_id, const std::string& date) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "SELECT id, meal_type, meal_name, calories, protein, created_at "
        "FROM nutrition WHERE user_id=? AND date=? ORDER BY created_at DESC",
        -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);

    crow::json::wvalue meals;
    std::vector<crow::json::wvalue> meal_list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        crow::json::wvalue meal;
        meal["id"] = sqlite3_column_int(stmt, 0);
        meal["meal_type"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        meal["meal_name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        meal["calories"] = sqlite3_column_int(stmt, 3);
        meal["protein"] = sqlite3_column_double(stmt, 4);
        meal["created_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        meal_list.push_back(std::move(meal));
    }

    sqlite3_finalize(stmt);
    return crow::response(meals);
}



crow::response deleteMeal(crow::SimpleApp&, sqlite3* db, int meal_id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "DELETE FROM nutrition WHERE id=?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, meal_id);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    return ok ? crow::response(200, "Deleted") : crow::response(500, "Failed");
}

crow::response updateMeal(crow::SimpleApp& app, sqlite3* db, int meal_id, const crow::request& req) {
    auto data = crow::json::load(req.body);
    if (!data) return crow::response(400, "Invalid JSON");

    std::string error;
    if (!validateMealData(data, error)) return crow::response(400, error);

    std::string meal_type = data["meal_type"].s();
    std::string meal_name = data["meal_name"].s();
    int calories = data["calories"].i();
    double protein = data.has("protein") ? data["protein"].d() : 0.0;

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "UPDATE nutrition SET meal_type=?, meal_name=?, calories=?, protein=? WHERE id=?",
        -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, meal_type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, meal_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, calories);
    sqlite3_bind_double(stmt, 4, protein);
    sqlite3_bind_int(stmt, 5, meal_id);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    return ok ? crow::response(200, "Updated") : crow::response(500, "Update failed");
}


crow::response clearDayMeals(crow::SimpleApp& app, sqlite3* db, int user_id, const std::string& date) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "DELETE FROM nutrition WHERE user_id=? AND date=?",
        -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    return ok ? crow::response(200, "Cleared") : crow::response(500, "Failed to clear meals");
}

crow::response getUserGoals(crow::SimpleApp &app, sqlite3 *db, int user_id)
{
    return crow::response();
}

crow::response updateUserGoals(crow::SimpleApp &app, sqlite3 *db, int user_id, const crow::request &req)
{
    return crow::response();
}

std::string getCurrentDate()
{
    return std::string();
}

std::string getCurrentDateTime()
{
    return std::string();
}

bool validateMealData(const crow::json::rvalue &data, std::string &error)
{
    return false;
}

bool validateGoalsData(const crow::json::rvalue &data, std::string &error)
{
    return false;
}
