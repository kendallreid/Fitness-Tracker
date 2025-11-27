#include <crow.h>
#include <sqlite3.h>
#include "../helper.h"
#include "../leaderboard.h"
#include "../invites.h"   // for getFriendships()
#include <vector>
#include <iostream>

// Get top users globally
std::vector<UserSimple> getTopUsers(sqlite3* db, int limit) {
    std::vector<UserSimple> users;
    const char* sql = "SELECT username, score FROM users ORDER BY score DESC LIMIT ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return users;
    sqlite3_bind_int(stmt, 1, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserSimple u;
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        u.score = sqlite3_column_int(stmt, 1);
        users.push_back(u);
    }

    sqlite3_finalize(stmt);
    return users;
}

// Get top friends for a user
std::vector<UserSimple> getTopFriends(sqlite3* db, int userId, int limit) {
    std::vector<UserSimple> users;
    std::vector<Friendship> friends = getFriendships(db, userId);

    if (friends.empty()) return users;

    std::string friendIds;
    for (const auto &f : friends) {
        int fid = (f.userId1 == userId) ? f.userId2 : f.userId1;
        friendIds += std::to_string(fid) + ",";
    }
    friendIds.pop_back(); // remove trailing comma

    std::string sql = "SELECT username, score FROM users WHERE id IN (" + friendIds + ") ORDER BY score DESC LIMIT ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return users;

    sqlite3_bind_int(stmt, 1, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserSimple u;
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        u.score = sqlite3_column_int(stmt, 1);
        users.push_back(u);
    }

    sqlite3_finalize(stmt);
    return users;
}

// Setup routes
void setupLeaderboardRoutes(crow::SimpleApp& app, sqlite3* db) {
    CROW_ROUTE(app, "/api/top-users").methods("GET"_method)([db](const crow::request& req, crow::response& res) {
        int limit = 3; // default
        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));
        if (limit > 100) limit = 100;

        bool friendsOnly = false;
        if (req.url_params.get("friends")) friendsOnly = std::string(req.url_params.get("friends")) == "1";

        std::vector<UserSimple> topUsers;

        if (friendsOnly) {
            // Get current user from cookie
            std::string cookieHeader = req.get_header_value("Cookie");
            std::string user_id_str = getCookieValue(cookieHeader, "user_id");
            if (user_id_str.empty()) {
                res.code = 401;
                res.write(R"({"error":"Unauthorized"})");
                res.end();
                return;
            }
            int userId = std::stoi(user_id_str);
            topUsers = getTopFriends(db, userId, limit);
        } else {
            topUsers = getTopUsers(db, limit);
        }

        crow::json::wvalue j;
        j["users"] = crow::json::wvalue::list();
        int i = 0;
        for (const auto &u : topUsers) {
            j["users"][i]["username"] = u.username;
            j["users"][i]["score"] = u.score;
            i++;
        }

        res.set_header("Content-Type", "application/json");
        res.write(j.dump());
        res.end();
    });
}
