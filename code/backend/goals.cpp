#include "calorie_tracker.h"
#include <sqlite3.h>

crow::response getUserGoals(crow::SimpleApp& app, sqlite3* db, int user_id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "SELECT daily_calorie_goal, daily_protein_goal FROM goals WHERE user_id=?",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, user_id);

    crow::json::wvalue result;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result["calorie_goal"] = sqlite3_column_int(stmt, 0);
        result["protein_goal"] = sqlite3_column_double(stmt, 1);
    } else {
        result["calorie_goal"] = 0;
        result["protein_goal"] = 0.0;
    }

    sqlite3_finalize(stmt);
    return crow::response(result);
}

crow::response updateUserGoals(crow::SimpleApp& app, sqlite3* db, int user_id, const crow::request& req) {
    auto data = crow::json::load(req.body);
    if (!data) return crow::response(400, "Invalid JSON");

    int calorie_goal = data["calorie_goal"].i();
    double protein_goal = data["protein_goal"].d();

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "REPLACE INTO goals (user_id, daily_calorie_goal, daily_protein_goal) VALUES (?, ?, ?)",
        -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, calorie_goal);
    sqlite3_bind_double(stmt, 3, protein_goal);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    return ok ? crow::response(200, "Goals Updated") : crow::response(500, "Failed to update goals");
}
