#include "crow.h"
#include "exercise.h"
#include "score_tracker.h"

bool addExercise(sqlite3 *db, const Exercise &e)
{
    // Prepare SQL statement for inserting a new exercise
    const char *sql = R"(
        INSERT INTO exercises (user_id, date, type, sets, reps, weight, duration, session_id, notes)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        std::cerr.flush();
        return false;
    }


    // Bind parameters
    sqlite3_bind_int(stmt, 1, e.user_id);
    sqlite3_bind_text(stmt, 2, e.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, e.type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, e.sets);
    sqlite3_bind_int(stmt, 5, e.reps);
    sqlite3_bind_double(stmt, 6, e.weight);
    sqlite3_bind_int(stmt, 7, e.duration);
    sqlite3_bind_int(stmt, 8, e.session_id);
    sqlite3_bind_text(stmt, 9, e.notes.c_str(), -1, SQLITE_STATIC);

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

std::vector<Exercise> getUserExercises(sqlite3 *db, int user_id)
{
    std::vector<Exercise> exercises;

    // Prepare statement
    const char *sql = R"(
        SELECT id, user_id, date, type, sets, reps, weight, duration, session_id, notes
        FROM exercises
        WHERE user_id = ?
        ORDER BY date DESC;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return exercises;
    }

    // Bind user_id parameter
    sqlite3_bind_int(stmt, 1, user_id);

    // execute the statement for all such exercises, set values
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Exercise e;
        e.id = sqlite3_column_int(stmt, 0);
        e.user_id = sqlite3_column_int(stmt, 1);
        const unsigned char* date = sqlite3_column_text(stmt, 2);
        e.date = date ? reinterpret_cast<const char*>(date) : "";
        const unsigned char* type = sqlite3_column_text(stmt, 3);
        e.type = type ? reinterpret_cast<const char*>(type) : "";
        e.sets = sqlite3_column_int(stmt, 4);
        e.reps = sqlite3_column_int(stmt, 5);
        e.weight = sqlite3_column_double(stmt, 6);
        e.duration = sqlite3_column_int(stmt, 7);
        e.session_id = sqlite3_column_int(stmt, 8);
        const unsigned char* notes = sqlite3_column_text(stmt, 9);
        e.notes = notes ? reinterpret_cast<const char*>(notes) : "";
        exercises.push_back(e);
    }

    sqlite3_finalize(stmt);
    return exercises;
}

std::vector<Exercise> getExercisesBySession(sqlite3 *db, int session_id)
{
    std::vector<Exercise> exercises;

    const char *sql = R"(
        SELECT id, user_id, date, type, sets, reps, weight, duration, session_id, notes
        FROM exercises
        WHERE session_id = ?
        ORDER BY date DESC;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return exercises;
    }

    sqlite3_bind_int(stmt, 1, session_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Exercise e;
        e.id = sqlite3_column_int(stmt, 0);
        e.user_id = sqlite3_column_int(stmt, 1);
        const unsigned char* date = sqlite3_column_text(stmt, 2);
        e.date = date ? reinterpret_cast<const char*>(date) : "";
        const unsigned char* type = sqlite3_column_text(stmt, 3);
        e.type = type ? reinterpret_cast<const char*>(type) : "";
        e.sets = sqlite3_column_int(stmt, 4);
        e.reps = sqlite3_column_int(stmt, 5);
        e.weight = sqlite3_column_double(stmt, 6);
        e.duration = sqlite3_column_int(stmt, 7);
        e.session_id = sqlite3_column_int(stmt, 8);
        const unsigned char* notes = sqlite3_column_text(stmt, 9);
        e.notes = notes ? reinterpret_cast<const char*>(notes) : "";
        exercises.push_back(e);
    }

    sqlite3_finalize(stmt);
    return exercises;
}

std::vector<Exercise> getUserExercisesByDate(sqlite3 *db, int user_id, const std::string &startDate, const std::string &endDate)
{
    std::vector<Exercise> exercises;

    const char *sql = R"(
        SELECT id, user_id, date, type, sets, reps, weight, duration, session_id, notes
        FROM exercises
        WHERE user_id = ? AND date BETWEEN ? AND ?
        ORDER BY date DESC, id DESC;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return exercises;
    }

    // Bind parameters
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, endDate.c_str(), -1, SQLITE_STATIC);

    // Execute the statement and populate the vector
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Exercise e;
        e.id = sqlite3_column_int(stmt, 0);
        e.user_id = sqlite3_column_int(stmt, 1);
        const unsigned char* text = sqlite3_column_text(stmt, 2);
        e.date = text ? reinterpret_cast<const char*>(text) : "";
        const unsigned char* type = sqlite3_column_text(stmt, 3);
        e.type = type ? reinterpret_cast<const char*>(type) : "";
        e.sets = sqlite3_column_int(stmt, 4);
        e.reps = sqlite3_column_int(stmt, 5);
        e.weight = sqlite3_column_double(stmt, 6);
        e.duration = sqlite3_column_int(stmt, 7);
        e.session_id = sqlite3_column_int(stmt, 8);
        const unsigned char* notes = sqlite3_column_text(stmt, 9);
        e.notes = notes ? reinterpret_cast<const char*>(notes) : "";
        exercises.push_back(e);
    }

    sqlite3_finalize(stmt);
    return exercises;
}

bool deleteExercise(sqlite3 *db, int exercise_id, int user_id)
{
    const char *sql = R"(
        DELETE FROM exercises
        WHERE id = ? AND user_id = ?;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, exercise_id);
    sqlite3_bind_int(stmt, 2, user_id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Failed to delete exercise: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

bool updateExercise(sqlite3 *db, const Exercise &e)
{
    const char *sql = R"(
        UPDATE exercises
        SET date = ?, type = ?, sets = ?, reps = ?, weight = ?, duration = ?, session_id = ?, notes = ?
        WHERE id = ? AND user_id = ?;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, e.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, e.type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, e.sets);
    sqlite3_bind_int(stmt, 4, e.reps);
    sqlite3_bind_double(stmt, 5, e.weight);
    sqlite3_bind_int(stmt, 6, e.duration);
    sqlite3_bind_int(stmt, 7, e.session_id);
    sqlite3_bind_text(stmt, 8, e.notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 9, e.id);
    sqlite3_bind_int(stmt, 10, e.user_id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Failed to update exercise: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

void registerExerciseRoutes(crow::SimpleApp& app, sqlite3* db)
{
    // --- Add Exercise ---
    CROW_ROUTE(app, "/api/exercises").methods("POST"_method)([db](const crow::request& req)
    {
        // Read user_id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int user_id = std::stoi(user_id_str);

        auto body = crow::json::load(req.body);
        if (!body)
            return makeError(400, "Invalid JSON");

        if (!body.has("date") || !body.has("type"))
            return makeError(400, "Missing required fields");

        Exercise e;
        e.user_id = user_id;
        e.date = body["date"].s();
        e.type = body["type"].s();
        e.sets = body.has("sets") ? body["sets"].i() : 0;
        e.reps = body.has("reps") ? body["reps"].i() : 0;
        e.weight = body.has("weight") ? body["weight"].d() : -1.0;
        e.duration = body.has("duration") ? body["duration"].i() : -1;
        e.notes = body.has("notes") ? body["notes"].s() : std::string("");
        e.session_id = body.has("session_id") ? body["session_id"].i() : 0;

        if (addExercise(db, e))
            return crow::response(201, crow::json::wvalue{{"message", "Exercise added successfully"}});
        else
            return makeError(500, "Failed to add exercise");
    });

    // --- Get Exercises ---
    CROW_ROUTE(app, "/api/exercises").methods("GET"_method)([db](const crow::request& req)
    {
        // Read user_id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int user_id = std::stoi(user_id_str);

        const char* session_id_str = req.url_params.get("session_id");
        std::vector<Exercise> exercises;

        if (session_id_str)
        {
            int session_id = std::stoi(session_id_str);
            exercises = getExercisesBySession(db, session_id);
        }
        else
        {
            const char* start_str = req.url_params.get("start");
            const char* end_str   = req.url_params.get("end");
            if (start_str && end_str)
                exercises = getUserExercisesByDate(db, user_id, start_str, end_str);
            else
                exercises = getUserExercises(db, user_id);
        }

        crow::json::wvalue res;
        crow::json::wvalue::list arr;

        for (const auto& e : exercises)
        {
            crow::json::wvalue item;
            item["id"] = e.id;
            item["user_id"] = e.user_id;
            item["date"] = e.date;
            item["type"] = e.type;
            item["sets"] = e.sets;
            item["reps"] = e.reps;
            item["weight"] = e.weight;
            item["duration"] = e.duration;
            item["notes"] = e.notes;
            item["session_id"] = e.session_id;

            arr.push_back(std::move(item));
        }

        res["exercises"] = std::move(arr);
        return crow::response(200, res);
    });

    // --- Update Exercise ---
    CROW_ROUTE(app, "/api/exercises").methods("PUT"_method)([db](const crow::request& req)
    {
        // Read user_id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int user_id = std::stoi(user_id_str);

        auto body = crow::json::load(req.body);
        if (!body)
            return makeError(400, "Invalid JSON");

        if (!body.has("id") || !body.has("date") || !body.has("type"))
            return makeError(400, "Missing required fields");

        Exercise e;
        e.id = body["id"].i();
        e.user_id = user_id;
        e.date = body["date"].s();
        e.type = body["type"].s();
        e.sets = body.has("sets") ? body["sets"].i() : 0;
        e.reps = body.has("reps") ? body["reps"].i() : 0;
        e.weight = body.has("weight") ? body["weight"].d() : -1.0;
        e.duration = body.has("duration") ? body["duration"].i() : -1;
        e.notes = body.has("notes") ? body["notes"].s() : std::string("");
        e.session_id = body.has("session_id") ? body["session_id"].i() : 0;

        if (updateExercise(db, e))
            return crow::response(200, crow::json::wvalue{{"message", "Exercise updated successfully"}});
        else
            return makeError(500, "Failed to update exercise");
    });

    // --- Delete Exercise ---
    CROW_ROUTE(app, "/api/exercises/<int>").methods("DELETE"_method)(
    [db](const crow::request& req, int exercise_id)
    {
        // Read user_id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int user_id = std::stoi(user_id_str);

        if (deleteExercise(db, exercise_id, user_id))
            return crow::response(200, crow::json::wvalue{{"message", "Exercise deleted successfully"}});
        else
            return makeError(404, "Exercise not found or not owned by user");
    });
}
