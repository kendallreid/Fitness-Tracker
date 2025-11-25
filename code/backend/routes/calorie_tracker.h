#pragma once
#include <crow.h>
#include <sqlite3.h>
#include <string>

void setupCalorieTrackerRoutes(crow::SimpleApp& app, sqlite3* db);

// Meal functions now match .cpp
crow::response addMeal(crow::SimpleApp& app, sqlite3* db, const crow::request& req);
crow::response getMeals(crow::SimpleApp& app, sqlite3* db, int user_id, const std::string& date);
crow::response updateMeal(crow::SimpleApp& app, sqlite3* db, int meal_id, const crow::request& req);
crow::response deleteMeal(crow::SimpleApp& app, sqlite3* db, int meal_id);
crow::response clearDayMeals(crow::SimpleApp& app, sqlite3* db, int user_id, const std::string& date);

// Goals
crow::response getUserGoals(crow::SimpleApp& app, sqlite3* db, int user_id);
crow::response updateUserGoals(crow::SimpleApp& app, sqlite3* db, int user_id, const crow::request& req);

// Weekly Summary
crow::response getDailySummary(crow::SimpleApp& app, sqlite3* db, int user_id, const std::string& date);
crow::response getWeeklySummary(crow::SimpleApp& app, sqlite3* db, int user_id);

// Utility
std::string getCurrentDate();
std::string getCurrentDateTime();
bool validateMealData(const crow::json::rvalue& data, std::string& error);
// bool validateGoalsData(const crow::json::rvalue& data, std::string& error);
