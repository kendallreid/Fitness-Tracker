#pragma once
#include <string>
#include <sodium.h>
#include <stdexcept>
#include <sqlite3.h>
#include <iostream>
#include "hash.h"
#include "../helper.h"

using namespace std;

struct Workout {
    int id;              // Primary key
    int user_id;         // Foreign key to users table
    std::string date;    // ISO format "YYYY-MM-DD"
    std::string type;    // Exercise type (e.g., "Bench Press", "Running")
    int sets;            // Optional (can be 0 for cardio)
    int reps;            // Optional
    double weight = -1;       // Optional (in kg or lbs)
    int duration = -1;        // Total duration (in minutes)
    std::string notes;   // Optional comments
};

void registerWorkoutRoutes(crow::SimpleApp& app, sqlite3* db);


