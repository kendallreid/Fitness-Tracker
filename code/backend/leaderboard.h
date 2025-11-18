#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <crow.h>
#include <sqlite3.h>
#include <vector>
#include <string>

// Backend Logic
struct User
{
    std::string username;
    int score;
};

// Routes
void setupGoalRoutes(crow::SimpleApp& app, sqlite3* db);
std::vector<User> getTopUsers(sqlite3* db, int limit);

#endif