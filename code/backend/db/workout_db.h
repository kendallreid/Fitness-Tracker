#pragma once
#include <vector>
#include "../routes/workout.h"
#include <sqlite3.h>


// Insert new workout
bool addWorkout(sqlite3* db, const Workout& w);

// Get all workouts for a user
std::vector<Workout> getUserWorkouts(sqlite3* db, int user_id);

// Get workouts filtered by date range (optional)
std::vector<Workout> getUserWorkoutsByDate(sqlite3* db, int user_id, const std::string& startDate, const std::string& endDate);

// Delete a specific workout by ID
bool deleteWorkout(sqlite3* db, int workout_id, int user_id);

// Update existing workout (optional)
bool updateWorkout(sqlite3* db, const Workout& w);
