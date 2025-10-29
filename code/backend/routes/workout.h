#pragma once
#include <string>
#include <sodium.h>
#include <stdexcept>
#include <iostream>
#include "hash.h"
#include "../helper.h"
#include <vector>
#include <sqlite3.h>

using namespace std;

struct Workout {
    int id;              // Primary key
    int user_id;         // Foreign key to users table
    int session_id;     // Foreign key to sessions table
    std::string date;    // ISO format "YYYY-MM-DD"
    std::string type;    // Exercise type (e.g., "Bench Press", "Running")
    int sets;            // Optional (can be 0 for cardio)
    int reps;            // Optional
    double weight = -1;       // Optional (in kg or lbs)
    int duration = -1;        // Total duration (in minutes)
    std::string notes;   // Optional comments
};

// Insert new workout
bool addWorkout(sqlite3* db, const Workout& w);

// Get all workouts for a user
std::vector<Workout> getUserWorkouts(sqlite3* db, int user_id);

// Get workouts by session id
std::vector<Workout> getWorkoutsBySession(sqlite3* db, int session_id);

// Get workouts filtered by date range (optional)
std::vector<Workout> getUserWorkoutsByDate(sqlite3* db, int user_id, const std::string& startDate, const std::string& endDate);

// Delete a specific workout by ID
bool deleteWorkout(sqlite3* db, int workout_id, int user_id);

// Update existing workout (optional)
bool updateWorkout(sqlite3* db, const Workout& w);

void registerWorkoutRoutes(crow::SimpleApp& app, sqlite3* db);


