#pragma once
#include <crow.h>
#include <sqlite3.h>
#include <string>

void setupSleepTrackerRoutes(crow::SimpleApp& app, sqlite3* db);

// Sleep functions now match .cpp
crow::response addSleep(crow::SimpleApp& app, sqlite3* db, const crow::request& req);
crow::response getSleeps(crow::SimpleApp& app, sqlite3* db, int user_id, const std::string& date);
crow::response updateSleep(crow::SimpleApp& app, sqlite3* db, int sleep_id, const crow::request& req);
crow::response deleteSleep(crow::SimpleApp&, sqlite3* db, int sleep_id);
crow::response clearWeeklySleeps(crow::SimpleApp& app, sqlite3* db, int user_id, const std::string& date);

// Goals
crow::response getUserGoals(crow::SimpleApp& app, sqlite3* db, int user_id);
crow::response updateUserGoals(crow::SimpleApp& app, sqlite3* db, int user_id, const crow::request& req);

// Utility
std::string getCurrentDate();
std::string getCurrentDateTime();
bool validateSleepData(const crow::json::rvalue& data, std::string& error);
