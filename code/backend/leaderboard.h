#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <crow.h>
#include <sqlite3.h>
#include <vector>
#include <string>

// Backend Logic
struct UserSimple
{
    std::string username;
    int score;
};

// Routes
void setupLeaderboardRoutes(crow::SimpleApp& app, sqlite3* db);
std::vector<UserSimple> getTopUsers(sqlite3* db, int limit);
std::vector<UserSimple> getTopFriends(sqlite3* db, int userId, int limit);

#endif
