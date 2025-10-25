#pragma once
#include <crow.h>
#include <sqlite3.h>
#include <string>
#include <vector>

struct MealEntry {
    int id;
    int user_id;
    std::string date;
    std::string meal_type;
    std::string meal_name;
    int calories;
    double protein;
    std::string created_at;
};

struct UserGoals {
    int id;
    int user_id;
    int daily_calorie_goal;
    double daily_protein_goal;
    std::string created_at;
    std::string updated_at;
};


// Function declarations
void setupCalorieTrackerRoutes(crow::SimpleApp& app, sqlite3* db);

// Meal management functions
crow::response addMeal(crow::SimpleApp& app, sqlite3* db, const crow::request& req);
crow::response getMeals(crow::SimpleApp& app, sqlite3* db, const crow::request& req);
crow::response updateMeal(crow::SimpleApp& app, sqlite3* db, const crow::request& req);
crow::response deleteMeal(crow::SimpleApp& app, sqlite3* db, const crow::request& req);
crow::response clearDayMeals(crow::SimpleApp& app, sqlite3* db, const crow::request& req);

// Goals management functions
crow::response getUserGoals(crow::SimpleApp& app, sqlite3* db, const crow::request& req);
crow::response updateUserGoals(crow::SimpleApp& app, sqlite3* db, const crow::request& req);


// Utility functions
std::string getCurrentDate();
std::string getCurrentDateTime();
bool validateMealData(const crow::json::rvalue& data, std::string& error);
bool validateGoalsData(const crow::json::rvalue& data, std::string& error);
