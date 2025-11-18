#include <crow.h>
#include <sqlite3.h>
#include "../leaderboard.h"
#include "../helper.h"
#include <vector>
#include <ctime>
#include <iostream>

std::vector<User> getTopUsers(sqlite3* db, int limit) {
    std::vector<User> users;
    const char* sql = "SELECT username, score FROM users ORDER BY score DESC LIMIT ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return users;
    }

    sqlite3_bind_int(stmt, 1, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        u.score = sqlite3_column_int(stmt, 1);
        users.push_back(u);
    }

    sqlite3_finalize(stmt);
    return users;
}

void setupLeaderboardRoutes(crow::SimpleApp& app, sqlite3* db)
{
    CROW_ROUTE(app, "/api/top-users").methods("GET"_method)([db](const crow::request& req, crow::response& res) {
        int limit = 3; // default
        if (req.url_params.get("limit")) {
            limit = std::stoi(req.url_params.get("limit"));
            if (limit > 100) limit = 100; 
        }

        auto topUsers = getTopUsers(db, limit);

        crow::json::wvalue j;
        j["users"] = crow::json::wvalue::list();

        int i = 0;
        for (const auto& u : topUsers) {
            j["users"][i]["username"] = u.username;
            j["users"][i]["score"] = u.score;
            i++;
        }

        res.set_header("Content-Type", "application/json");
        res.write(j.dump());
        res.end();
    });
}