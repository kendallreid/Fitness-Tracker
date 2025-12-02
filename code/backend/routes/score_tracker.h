#ifndef SCORE_TRACKER_H
#define SCORE_TRACKER_H

#include "../helper.h"

void addPoints(int userID, int points, sqlite3* db);
void addGoalPoints(int goalID, int points, sqlite3* db);
void removePoints(int userID, int points, sqlite3* db);

#endif //SCORE_TRACKER_H