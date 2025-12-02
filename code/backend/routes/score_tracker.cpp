
#include "../helper.h"
#include "score_tracker.h"

int fetchPoints(int userID, sqlite3* db)
{
    int points = 0;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "SELECT score "
        "FROM users WHERE id=?",
        -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, userID);

    if (sqlite3_step(stmt) == SQLITE_ROW) points = sqlite3_column_int(stmt, 0);
    
    sqlite3_finalize(stmt);
    return points;
}

void addPoints(int userID, int points, sqlite3* db)
{
    int newScore = fetchPoints(userID, db) + points;
    const char *sql = R"(
        UPDATE users
        SET score = ?
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,sql,-1, &stmt, nullptr);
    
    sqlite3_bind_int(stmt, 1, newScore);
    sqlite3_bind_int(stmt, 2, userID);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "SQLite error: " << sqlite3_errmsg(db) << "\nSQL: " << sql << std::endl;
        std::cerr << "[DEBUG] SCORE UPDATE FAILED — values were:\n";
        std::cerr << "  score           = " << newScore << "\n";
        std::cerr << "  user_id         = " << userID << "\n";
        sqlite3_finalize(stmt);
    }

    sqlite3_finalize(stmt);
}

void removePoints(int userID, int points, sqlite3* db)
{
    addPoints(userID,0-points, db);
}

int getUserIDforGoal(int goalID, sqlite3* db)
{
    int userID = -1;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "SELECT user_id "
        "FROM user_goals WHERE id=?",
        -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, goalID);

    if (sqlite3_step(stmt) == SQLITE_ROW) userID = sqlite3_column_int(stmt, 0);
    
    sqlite3_finalize(stmt);
    return userID;
}

void addGoalPoints(int goalID, int points, sqlite3* db)
{
    addPoints(getUserIDforGoal(goalID, db), points, db);
}

