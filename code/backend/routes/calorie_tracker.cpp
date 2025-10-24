#include "calorie_tracker.h"
#include "../helper.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <json/json.h>

void setupCalorieTrackerRoutes(crow::SimpleApp& app, sqlite3* db) {
    CROW_ROUTE(app, "/calorie-tracker")
    ([]{
        return serveFile("code/frontend/CalorieTracker.html", "text/html");
    });

    CROW_ROUTE(app, "/api/meals").methods("POST"_method)
    ([&app, db](const crow::request& req) {
        return addMeal(app, db, req);
    });

    CROW_ROUTE(app, "/api/meals/<int>/<string>").methods("GET"_method)
    ([&app, db](int user_id, const std::string& date) {
        crow::request req;
        req.url = "/api/meals/" + std::to_string(user_id) + "/" + date;
        return getMeals(app, db, req);
    });

    CROW_ROUTE(app, "/api/meals/<int>").methods("PUT"_method)
    ([&app, db](int meal_id, const crow::request& req) {
        crow::request modified_req = req;
        modified_req.url = "/api/meals/" + std::to_string(meal_id);
        return updateMeal(app, db, modified_req);
    });

    CROW_ROUTE(app, "/api/meals/<int>").methods("DELETE"_method)
    ([&app, db](int meal_id, const crow::request& req) {
        crow::request modified_req = req;
        modified_req.url = "/api/meals/" + std::to_string(meal_id);
        return deleteMeal(app, db, modified_req);
    });

    CROW_ROUTE(app, "/api/meals/clear/<int>/<string>").methods("DELETE"_method)
    ([&app, db](int user_id, const std::string& date) {
        crow::request req;
        req.url = "/api/meals/clear/" + std::to_string(user_id) + "/" + date;
        return clearDayMeals(app, db, req);
    });

    CROW_ROUTE(app, "/api/goals/<int>").methods("GET"_method)
    ([&app, db](int user_id) {
        crow::request req;
        req.url = "/api/goals/" + std::to_string(user_id);
        return getUserGoals(app, db, req);
    });

    CROW_ROUTE(app, "/api/goals/<int>").methods("PUT"_method)
    ([&app, db](int user_id, const crow::request& req) {
        crow::request modified_req = req;
        modified_req.url = "/api/goals/" + std::to_string(user_id);
        return updateUserGoals(app, db, modified_req);
    });

}

crow::response addMeal(crow::SimpleApp& app, sqlite3* db, const crow::request& req) {
    try {
        Json::Value json_data;
        Json::Reader reader;
        
        if (!reader.parse(req.body, json_data)) {
            crow::response res(400);
            res.body = "{\"error\":\"Invalid JSON\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        std::string error;
        if (!validateMealData(json_data, error)) {
            crow::response res(400);
            res.body = "{\"error\":\"" + error + "\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        int user_id = json_data["user_id"].asInt();
        std::string date = json_data.get("date", getCurrentDate()).asString();
        std::string meal_type = json_data["meal_type"].asString();
        std::string meal_name = json_data["meal_name"].asString();
        int calories = json_data["calories"].asInt();
        double protein = json_data.get("protein", 0.0).asDouble();
        std::string created_at = getCurrentDateTime();

        std::string sql = "INSERT INTO nutrition (user_id, date, meal_type, meal_name, calories, protein, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            crow::response res(500);
            res.body = "{\"error\":\"Database preparation failed\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, meal_type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, meal_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, calories);
        sqlite3_bind_double(stmt, 6, protein);
        sqlite3_bind_text(stmt, 7, created_at.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            crow::response res(500);
            res.body = "{\"error\":\"Failed to insert meal\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        int meal_id = sqlite3_last_insert_rowid(db);
        sqlite3_finalize(stmt);

        crow::response res(201);
        res.body = "{\"message\":\"Meal added successfully\",\"meal_id\":" + std::to_string(meal_id) + "}";
        res.set_header("Content-Type", "application/json");
        return res;

    } catch (const std::exception& e) {
        crow::response res(500);
        res.body = "{\"error\":\"Internal server error: " + std::string(e.what()) + "\"}";
        res.set_header("Content-Type", "application/json");
        return res;
    }
}

crow::response getMeals(crow::SimpleApp& app, sqlite3* db, const crow::request& req) {
    try {
        std::string url = req.url;
        size_t last_slash = url.find_last_of('/');
        std::string date = url.substr(last_slash + 1);
        size_t second_last_slash = url.substr(0, last_slash).find_last_of('/');
        int user_id = std::stoi(url.substr(second_last_slash + 1, last_slash - second_last_slash - 1));

        std::string sql = "SELECT id, user_id, date, meal_type, meal_name, calories, protein, created_at FROM nutrition WHERE user_id = ? AND date = ? ORDER BY created_at DESC";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            crow::response res(500);
            res.body = "{\"error\":\"Database preparation failed\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);

        Json::Value meals(Json::arrayValue);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Json::Value meal;
            meal["id"] = sqlite3_column_int(stmt, 0);
            meal["user_id"] = sqlite3_column_int(stmt, 1);
            meal["date"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            meal["meal_type"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            meal["meal_name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            meal["calories"] = sqlite3_column_int(stmt, 5);
            meal["protein"] = sqlite3_column_double(stmt, 6);
            meal["created_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            meals.append(meal);
        }

        sqlite3_finalize(stmt);

        Json::StreamWriterBuilder builder;
        std::string response_body = Json::writeString(builder, meals);

        crow::response res(200);
        res.body = response_body;
        res.set_header("Content-Type", "application/json");
        return res;

    } catch (const std::exception& e) {
        crow::response res(500);
        res.body = "{\"error\":\"Internal server error: " + std::string(e.what()) + "\"}";
        res.set_header("Content-Type", "application/json");
        return res;
    }
}

crow::response updateMeal(crow::SimpleApp& app, sqlite3* db, const crow::request& req) {
    try {
        std::string url = req.url;
        size_t last_slash = url.find_last_of('/');
        int meal_id = std::stoi(url.substr(last_slash + 1));

        Json::Value json_data;
        Json::Reader reader;
        
        if (!reader.parse(req.body, json_data)) {
            crow::response res(400);
            res.body = "{\"error\":\"Invalid JSON\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        std::string error;
        if (!validateMealData(json_data, error)) {
            crow::response res(400);
            res.body = "{\"error\":\"" + error + "\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        std::string meal_type = json_data["meal_type"].asString();
        std::string meal_name = json_data["meal_name"].asString();
        int calories = json_data["calories"].asInt();
        double protein = json_data.get("protein", 0.0).asDouble();

        std::string sql = "UPDATE nutrition SET meal_type = ?, meal_name = ?, calories = ?, protein = ? WHERE id = ?";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            crow::response res(500);
            res.body = "{\"error\":\"Database preparation failed\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_bind_text(stmt, 1, meal_type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, meal_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, calories);
        sqlite3_bind_double(stmt, 4, protein);
        sqlite3_bind_int(stmt, 5, meal_id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            crow::response res(500);
            res.body = "{\"error\":\"Failed to update meal\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_finalize(stmt);

        crow::response res(200);
        res.body = "{\"message\":\"Meal updated successfully\"}";
        res.set_header("Content-Type", "application/json");
        return res;

    } catch (const std::exception& e) {
        crow::response res(500);
        res.body = "{\"error\":\"Internal server error: " + std::string(e.what()) + "\"}";
        res.set_header("Content-Type", "application/json");
        return res;
    }
}

crow::response deleteMeal(crow::SimpleApp& app, sqlite3* db, const crow::request& req) {
    try {
        std::string url = req.url;
        size_t last_slash = url.find_last_of('/');
        int meal_id = std::stoi(url.substr(last_slash + 1));

        std::string sql = "DELETE FROM nutrition WHERE id = ?";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            crow::response res(500);
            res.body = "{\"error\":\"Database preparation failed\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_bind_int(stmt, 1, meal_id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            crow::response res(500);
            res.body = "{\"error\":\"Failed to delete meal\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_finalize(stmt);

        crow::response res(200);
        res.body = "{\"message\":\"Meal deleted successfully\"}";
        res.set_header("Content-Type", "application/json");
        return res;

    } catch (const std::exception& e) {
        crow::response res(500);
        res.body = "{\"error\":\"Internal server error: " + std::string(e.what()) + "\"}";
        res.set_header("Content-Type", "application/json");
        return res;
    }
}

crow::response clearDayMeals(crow::SimpleApp& app, sqlite3* db, const crow::request& req) {
    try {
        std::string url = req.url;
        size_t last_slash = url.find_last_of('/');
        std::string date = url.substr(last_slash + 1);
        size_t second_last_slash = url.substr(0, last_slash).find_last_of('/');
        int user_id = std::stoi(url.substr(second_last_slash + 1, last_slash - second_last_slash - 1));

        std::string sql = "DELETE FROM nutrition WHERE user_id = ? AND date = ?";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            crow::response res(500);
            res.body = "{\"error\":\"Database preparation failed\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            crow::response res(500);
            res.body = "{\"error\":\"Failed to clear meals\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_finalize(stmt);

        crow::response res(200);
        res.body = "{\"message\":\"All meals cleared for the day\"}";
        res.set_header("Content-Type", "application/json");
        return res;

    } catch (const std::exception& e) {
        crow::response res(500);
        res.body = "{\"error\":\"Internal server error: " + std::string(e.what()) + "\"}";
        res.set_header("Content-Type", "application/json");
        return res;
    }
}

crow::response getUserGoals(crow::SimpleApp& app, sqlite3* db, const crow::request& req) {
    try {
        std::string url = req.url;
        size_t last_slash = url.find_last_of('/');
        int user_id = std::stoi(url.substr(last_slash + 1));

        std::string sql = "SELECT id, user_id, daily_calorie_goal, daily_protein_goal, created_at, updated_at FROM user_goals WHERE user_id = ?";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            crow::response res(500);
            res.body = "{\"error\":\"Database preparation failed\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_bind_int(stmt, 1, user_id);

        Json::Value goals;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            goals["id"] = sqlite3_column_int(stmt, 0);
            goals["user_id"] = sqlite3_column_int(stmt, 1);
            goals["daily_calorie_goal"] = sqlite3_column_int(stmt, 2);
            goals["daily_protein_goal"] = sqlite3_column_double(stmt, 3);
            goals["created_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            goals["updated_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        } else {
            goals["user_id"] = user_id;
            goals["daily_calorie_goal"] = 2000;
            goals["daily_protein_goal"] = 150.0;
            goals["created_at"] = getCurrentDateTime();
            goals["updated_at"] = getCurrentDateTime();
        }

        sqlite3_finalize(stmt);

        Json::StreamWriterBuilder builder;
        std::string response_body = Json::writeString(builder, goals);

        crow::response res(200);
        res.body = response_body;
        res.set_header("Content-Type", "application/json");
        return res;

    } catch (const std::exception& e) {
        crow::response res(500);
        res.body = "{\"error\":\"Internal server error: " + std::string(e.what()) + "\"}";
        res.set_header("Content-Type", "application/json");
        return res;
    }
}

crow::response updateUserGoals(crow::SimpleApp& app, sqlite3* db, const crow::request& req) {
    try {
        std::string url = req.url;
        size_t last_slash = url.find_last_of('/');
        int user_id = std::stoi(url.substr(last_slash + 1));

        Json::Value json_data;
        Json::Reader reader;
        
        if (!reader.parse(req.body, json_data)) {
            crow::response res(400);
            res.body = "{\"error\":\"Invalid JSON\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        std::string error;
        if (!validateGoalsData(json_data, error)) {
            crow::response res(400);
            res.body = "{\"error\":\"" + error + "\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        int daily_calorie_goal = json_data["daily_calorie_goal"].asInt();
        double daily_protein_goal = json_data["daily_protein_goal"].asDouble();
        std::string updated_at = getCurrentDateTime();

        std::string check_sql = "SELECT id FROM user_goals WHERE user_id = ?";
        sqlite3_stmt* check_stmt;
        if (sqlite3_prepare_v2(db, check_sql.c_str(), -1, &check_stmt, nullptr) != SQLITE_OK) {
            crow::response res(500);
            res.body = "{\"error\":\"Database preparation failed\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_bind_int(check_stmt, 1, user_id);
        bool goals_exist = (sqlite3_step(check_stmt) == SQLITE_ROW);
        sqlite3_finalize(check_stmt);

        std::string sql;
        if (goals_exist) {
            sql = "UPDATE user_goals SET daily_calorie_goal = ?, daily_protein_goal = ?, updated_at = ? WHERE user_id = ?";
        } else {
            sql = "INSERT INTO user_goals (user_id, daily_calorie_goal, daily_protein_goal, created_at, updated_at) VALUES (?, ?, ?, ?, ?)";
        }
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            crow::response res(500);
            res.body = "{\"error\":\"Database preparation failed\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        if (goals_exist) {
            sqlite3_bind_int(stmt, 1, daily_calorie_goal);
            sqlite3_bind_double(stmt, 2, daily_protein_goal);
            sqlite3_bind_text(stmt, 3, updated_at.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 4, user_id);
        } else {
            sqlite3_bind_int(stmt, 1, user_id);
            sqlite3_bind_int(stmt, 2, daily_calorie_goal);
            sqlite3_bind_double(stmt, 3, daily_protein_goal);
            sqlite3_bind_text(stmt, 4, updated_at.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, updated_at.c_str(), -1, SQLITE_STATIC);
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            crow::response res(500);
            res.body = "{\"error\":\"Failed to update goals\"}";
            res.set_header("Content-Type", "application/json");
            return res;
        }

        sqlite3_finalize(stmt);

        crow::response res(200);
        res.body = "{\"message\":\"Goals updated successfully\"}";
        res.set_header("Content-Type", "application/json");
        return res;

    } catch (const std::exception& e) {
        crow::response res(500);
        res.body = "{\"error\":\"Internal server error: " + std::string(e.what()) + "\"}";
        res.set_header("Content-Type", "application/json");
        return res;
    }
}


// Utility functions
std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool validateMealData(const Json::Value& data, std::string& error) {
    if (!data.isMember("user_id") || !data["user_id"].isInt()) {
        error = "user_id is required and must be an integer";
        return false;
    }
    
    if (!data.isMember("meal_name") || !data["meal_name"].isString() || data["meal_name"].asString().empty()) {
        error = "meal_name is required";
        return false;
    }
    
    if (!data.isMember("calories") || !data["calories"].isInt() || data["calories"].asInt() <= 0) {
        error = "calories is required and must be a positive integer";
        return false;
    }
    
    if (data["calories"].asInt() > 10000) {
        error = "calories value seems too high (max 10000)";
        return false;
    }
    
    if (!data.isMember("meal_type") || !data["meal_type"].isString() || data["meal_type"].asString().empty()) {
        error = "meal_type is required";
        return false;
    }
    
    if (data.isMember("protein") && (!data["protein"].isNumeric() || data["protein"].asDouble() < 0)) {
        error = "protein must be a not a negative number";
        return false;
    }
    
    return true;
}

bool validateGoalsData(const Json::Value& data, std::string& error) {
    if (!data.isMember("daily_calorie_goal") || !data["daily_calorie_goal"].isInt() || data["daily_calorie_goal"].asInt() <= 0) {
        error = "daily_calorie_goal is required and must be a positive";
        return false;
    }
    
    if (data["daily_calorie_goal"].asInt() > 10000) {
        error = "daily_calorie_goal seems too high (max 10000)";
        return false;
    }
    
    if (!data.isMember("daily_protein_goal") || !data["daily_protein_goal"].isNumeric() || data["daily_protein_goal"].asDouble() < 0) {
        error = "daily_protein_goal is required and must be a non-negative number";
        return false;
    }
    
    if (data["daily_protein_goal"].asDouble() > 1000) {
        error = "daily_protein_goal seems too high (max 1000)";
        return false;
    }
    
    return true;
}

