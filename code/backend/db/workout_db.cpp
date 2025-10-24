#include "workout_db.h"

bool addWorkout(sqlite3 *db, const Workout &w)
{
    // Prepare SQL statement for inserting a new workout
    const char *sql = R"(
        INSERT INTO workouts (user_id, date, type, sets, reps, weight, duration, notes)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        std::cerr.flush();
        return false;
    }


    // Bind parameters
    sqlite3_bind_int(stmt, 1, w.user_id);
    sqlite3_bind_text(stmt, 2, w.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, w.type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, w.sets);
    sqlite3_bind_int(stmt, 5, w.reps);
    sqlite3_bind_double(stmt, 6, w.weight);
    sqlite3_bind_int(stmt, 7, w.duration);
    sqlite3_bind_text(stmt, 8, w.notes.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) 
    {
        std::string err = sqlite3_errmsg(db);
        std::cerr << "Failed to add workout: " << err << std::endl;
        std::cerr.flush();

        // Also print to Crow log so you always see it:
        CROW_LOG_ERROR << "SQLite error: " << err;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);

    return success;
}

std::vector<Workout> getUserWorkouts(sqlite3 *db, int user_id)
{
    std::vector<Workout> workouts;

    // Prepare statement
    const char *sql = R"(
        SELECT id, user_id, date, type, sets, reps, weight, duration, notes
        FROM workouts
        WHERE user_id = ?
        ORDER BY date DESC;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return workouts;
    }

    // Bind user_id parameter
    sqlite3_bind_int(stmt, 1, user_id);

    // execute the statement for all such workouts, set values
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Workout w;
        w.id = sqlite3_column_int(stmt, 0);
        w.user_id = sqlite3_column_int(stmt, 1);
        w.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        w.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        w.sets = sqlite3_column_int(stmt, 4);
        w.reps = sqlite3_column_int(stmt, 5);
        w.weight = sqlite3_column_double(stmt, 6);
        w.duration = sqlite3_column_int(stmt, 7);
        w.notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        workouts.push_back(w);
    }

    sqlite3_finalize(stmt);
    return workouts;
}

std::vector<Workout> getUserWorkoutsByDate(sqlite3 *db, int user_id, const std::string &startDate, const std::string &endDate)
{
    std::vector<Workout> workouts;

    const char *sql = R"(
        SELECT id, user_id, date, type, sets, reps, weight, duration, notes
        FROM workouts
        WHERE user_id = ? AND date BETWEEN ? AND ?
        ORDER BY date DESC;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return workouts;
    }

    // Bind parameters
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, endDate.c_str(), -1, SQLITE_STATIC);

    // Execute the statement and populate the vector
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Workout w;
        w.id = sqlite3_column_int(stmt, 0);
        w.user_id = sqlite3_column_int(stmt, 1);
        w.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        w.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        w.sets = sqlite3_column_int(stmt, 4);
        w.reps = sqlite3_column_int(stmt, 5);
        w.weight = sqlite3_column_double(stmt, 6);
        w.duration = sqlite3_column_int(stmt, 7);
        w.notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        workouts.push_back(w);
    }

    sqlite3_finalize(stmt);
    return workouts;
}

bool deleteWorkout(sqlite3 *db, int workout_id, int user_id)
{
    const char *sql = R"(
        DELETE FROM workouts
        WHERE id = ? AND user_id = ?;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, workout_id);
    sqlite3_bind_int(stmt, 2, user_id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Failed to delete workout: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

bool updateWorkout(sqlite3 *db, const Workout &w)
{
    const char *sql = R"(
        UPDATE workouts
        SET date = ?, type = ?, sets = ?, reps = ?, weight = ?, duration = ?, notes = ?
        WHERE id = ? AND user_id = ?;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, w.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, w.type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, w.sets);
    sqlite3_bind_int(stmt, 4, w.reps);
    sqlite3_bind_double(stmt, 5, w.weight);
    sqlite3_bind_int(stmt, 6, w.duration);
    sqlite3_bind_text(stmt, 7, w.notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, w.id);
    sqlite3_bind_int(stmt, 9, w.user_id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Failed to update workout: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}