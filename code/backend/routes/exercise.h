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

struct Exercise {
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

// Insert new exercise
bool addExercise(sqlite3* db, const Exercise& w);

// Get all exercises for a user
std::vector<Exercise> getUserExercises(sqlite3* db, int user_id);

// Get exercises by session id
std::vector<Exercise> getExercisesBySession(sqlite3* db, int session_id);

// Get exercises filtered by date range (optional)
std::vector<Exercise> getUserExercisesByDate(sqlite3* db, int user_id, const std::string& startDate, const std::string& endDate);

// Delete a specific exercise by ID
bool deleteExercise(sqlite3* db, int exercise_id, int user_id);

// Update existing exercise (optional)
bool updateExercise(sqlite3* db, const Exercise& w);

void registerExerciseRoutes(crow::SimpleApp& app, sqlite3* db);


