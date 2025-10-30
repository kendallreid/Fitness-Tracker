#ifndef GOALTRACKER_H
#define GOALTRACKER_H

#include <crow.h>
#include <sqlite3.h>
#include <vector>
#include <string>

struct Goal {
    int id;
    int user_id;
    std::string goal_name;
    double target_value;
    std::string start_date;
    std::string end_date;
    std::string status;
    double total_progress;
};

// Backend logic
std::vector<Goal> getAllGoals(sqlite3* db, int user_id, const std::string& status_filter);
bool addGoal(sqlite3* db, int user_id, const std::string& goal_name,
             double target_value, const std::string& start_date, const std::string& end_date);
bool addGoalProgress(sqlite3* db, int goal_id, double value);
double getGoalTotalProgress(sqlite3* db, int goal_id);


// Routes
void setupGoalRoutes(crow::SimpleApp& app, sqlite3* db);

#endif
