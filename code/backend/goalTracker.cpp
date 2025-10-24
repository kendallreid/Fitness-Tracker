#include "goalTracker.h"
#include <iostream>

std::vector<Goal> getAllGoals(sqlite3* db) {
    std::vector<Goal> goals;

    const char* sql = "SELECT id, user_id, goal_name, target_value, start_date, end_date FROM goals;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare getAllGoals: " << sqlite3_errmsg(db) << std::endl;
        return goals;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Goal g;
        g.id = sqlite3_column_int(stmt, 0);
        g.user_id = sqlite3_column_int(stmt, 1);
        g.goal_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        g.target_value = sqlite3_column_double(stmt, 3);
        g.start_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        // end_date can be NULL in DB
        const unsigned char* endText = sqlite3_column_text(stmt, 5);
        g.end_date = endText ? reinterpret_cast<const char*>(endText) : "";

        goals.push_back(std::move(g));
    }

    sqlite3_finalize(stmt);
    return goals;
}

bool addGoal(sqlite3* db, int user_id, const std::string& goal_name,
             double target_value, const std::string& start_date, const std::string& end_date) 
{
    const char* sql = "INSERT INTO goals (user_id, goal_name, target_value, start_date, end_date) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare addGoal: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, goal_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, target_value);
    sqlite3_bind_text(stmt, 4, start_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, end_date.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success)
        std::cerr << "Error inserting goal: " << sqlite3_errmsg(db) << std::endl;

    sqlite3_finalize(stmt);
    return success;
}

bool addGoalProgress(sqlite3* db, int goal_id, double value) {
    const char* sql = "INSERT INTO goal_progress (goal_id, date, progress_value) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare addGoalProgress: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string today = ""; // get YYYY-MM-DD string for today
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    today = buf;

    sqlite3_bind_int(stmt, 1, goal_id);
    sqlite3_bind_text(stmt, 2, today.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, value);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success)
        std::cerr << "Error inserting goal progress: " << sqlite3_errmsg(db) << std::endl;

    sqlite3_finalize(stmt);
    return success;
}
