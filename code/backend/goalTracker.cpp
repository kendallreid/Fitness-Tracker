#include "goalTracker.h"
#include "score_tracker.h"
#include <iostream>

std::vector<Goal> getAllGoals(sqlite3* db, int user_id, const std::string& status_filter) {
    std::vector<Goal> goals;

    const char* sql;
    sqlite3_stmt* stmt;

    if (status_filter == "active") {
        sql = "SELECT id, user_id, goal_name, target_value, start_date, end_date, status FROM goals WHERE user_id = ? AND status = 'active' ORDER BY start_date DESC;";
    } else if (status_filter == "completed") {
        sql = "SELECT id, user_id, goal_name, target_value, start_date, end_date, status FROM goals WHERE user_id = ? AND status = 'completed' ORDER BY start_date DESC;";
    } else {
        sql = "SELECT id, user_id, goal_name, target_value, start_date, end_date, status FROM goals WHERE user_id = ? ORDER BY start_date DESC;";
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare getAllGoals: " << sqlite3_errmsg(db) << std::endl;
        return goals;
    }

    sqlite3_bind_int(stmt, 1, user_id);

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

        const unsigned char* statusText = sqlite3_column_text(stmt, 6);
        g.status = statusText ? reinterpret_cast<const char*>(statusText) : "active";

        g.total_progress = getGoalTotalProgress(db, g.id);
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

    if (success) addPoints(user_id, 5, db);
    return success;
}

bool addGoalProgress(sqlite3* db, int goal_id, double value) {
    const char* checkSql = "SELECT status FROM goals WHERE id = ?;";
    sqlite3_stmt* checkStmt;
    if (sqlite3_prepare_v2(db, checkSql, -1, &checkStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(checkStmt, 1, goal_id);
        if (sqlite3_step(checkStmt) == SQLITE_ROW) {
            const unsigned char* statusText = sqlite3_column_text(checkStmt, 0);
            std::string status = statusText ? reinterpret_cast<const char*>(statusText) : "active";
            sqlite3_finalize(checkStmt);
            if (status == "completed") {
                std::cerr << "Goal already completed, cannot add progress.\n";
                return false;
            }
        }
    } else {
        sqlite3_finalize(checkStmt);
        return false;
    }

    const char* sql = "INSERT INTO goal_progress (goal_id, date, progress_value) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare addGoalProgress: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // today's date
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    std::string today = buf;

    sqlite3_bind_int(stmt, 1, goal_id);
    sqlite3_bind_text(stmt, 2, today.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, value);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success)
        std::cerr << "Error inserting goal progress: " << sqlite3_errmsg(db) << std::endl;

    sqlite3_finalize(stmt);

    return success;
}

double getGoalTotalProgress(sqlite3* db, int goal_id) {
    const char* sql = "SELECT SUM(progress_value) FROM goal_progress WHERE goal_id = ?;";
    sqlite3_stmt* stmt;
    double total = 0.0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return 0.0;

    sqlite3_bind_int(stmt, 1, goal_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return total;
}