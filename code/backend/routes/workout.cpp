#include "crow.h"
#include "workout.h"

bool addWorkout(sqlite3 *db, const Workout &w)
{
    // Prepare SQL statement for inserting a new workout
    const char *sql = R"(
        INSERT INTO workouts (user_id, date, type, sets, reps, weight, duration, session_id, notes)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);
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
    sqlite3_bind_int(stmt, 8, w.session_id);
    sqlite3_bind_text(stmt, 9, w.notes.c_str(), -1, SQLITE_STATIC);

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
        SELECT id, user_id, date, type, sets, reps, weight, duration, session_id, notes
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
        const unsigned char* date = sqlite3_column_text(stmt, 2);
        w.date = date ? reinterpret_cast<const char*>(date) : "";
        const unsigned char* type = sqlite3_column_text(stmt, 3);
        w.type = type ? reinterpret_cast<const char*>(type) : "";
        w.sets = sqlite3_column_int(stmt, 4);
        w.reps = sqlite3_column_int(stmt, 5);
        w.weight = sqlite3_column_double(stmt, 6);
        w.duration = sqlite3_column_int(stmt, 7);
        w.session_id = sqlite3_column_int(stmt, 8);
        const unsigned char* notes = sqlite3_column_text(stmt, 9);
        w.notes = notes ? reinterpret_cast<const char*>(notes) : "";
        workouts.push_back(w);
    }

    sqlite3_finalize(stmt);
    return workouts;
}

std::vector<Workout> getWorkoutsBySession(sqlite3 *db, int session_id)
{
    std::vector<Workout> workouts;

    const char *sql = R"(
        SELECT id, user_id, date, type, sets, reps, weight, duration, session_id, notes
        FROM workouts
        WHERE session_id = ?
        ORDER BY date DESC;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return workouts;
    }

    sqlite3_bind_int(stmt, 1, session_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Workout w;
        w.id = sqlite3_column_int(stmt, 0);
        w.user_id = sqlite3_column_int(stmt, 1);
        const unsigned char* date = sqlite3_column_text(stmt, 2);
        w.date = date ? reinterpret_cast<const char*>(date) : "";
        const unsigned char* type = sqlite3_column_text(stmt, 3);
        w.type = type ? reinterpret_cast<const char*>(type) : "";
        w.sets = sqlite3_column_int(stmt, 4);
        w.reps = sqlite3_column_int(stmt, 5);
        w.weight = sqlite3_column_double(stmt, 6);
        w.duration = sqlite3_column_int(stmt, 7);
        w.session_id = sqlite3_column_int(stmt, 8);
        const unsigned char* notes = sqlite3_column_text(stmt, 9);
        w.notes = notes ? reinterpret_cast<const char*>(notes) : "";
        workouts.push_back(w);
    }

    sqlite3_finalize(stmt);
    return workouts;
}

std::vector<Workout> getUserWorkoutsByDate(sqlite3 *db, int user_id, const std::string &startDate, const std::string &endDate)
{
    std::vector<Workout> workouts;

    const char *sql = R"(
        SELECT id, user_id, date, type, sets, reps, weight, duration, session_id, notes
        FROM workouts
        WHERE user_id = ? AND date BETWEEN ? AND ?
        ORDER BY date DESC, id DESC;
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
        const unsigned char* text = sqlite3_column_text(stmt, 2);
        w.date = text ? reinterpret_cast<const char*>(text) : "";
        const unsigned char* type = sqlite3_column_text(stmt, 3);
        w.type = type ? reinterpret_cast<const char*>(type) : "";
        w.sets = sqlite3_column_int(stmt, 4);
        w.reps = sqlite3_column_int(stmt, 5);
        w.weight = sqlite3_column_double(stmt, 6);
        w.duration = sqlite3_column_int(stmt, 7);
        w.session_id = sqlite3_column_int(stmt, 8);
        const unsigned char* notes = sqlite3_column_text(stmt, 9);
        w.notes = notes ? reinterpret_cast<const char*>(notes) : "";
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
        SET date = ?, type = ?, sets = ?, reps = ?, weight = ?, duration = ?, session_id = ?, notes = ?
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
    sqlite3_bind_int(stmt, 7, w.session_id);
    sqlite3_bind_text(stmt, 8, w.notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 9, w.id);
    sqlite3_bind_int(stmt, 10, w.user_id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Failed to update workout: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

void registerWorkoutRoutes(crow::SimpleApp& app, sqlite3* db)
{
    // --- Add Workout ---
    CROW_ROUTE(app, "/api/workouts").methods("POST"_method)([db](const crow::request& req)
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

        Workout w;
        w.user_id = user_id;
        w.date = body["date"].s();
        w.type = body["type"].s();
        w.sets = body.has("sets") ? body["sets"].i() : 0;
        w.reps = body.has("reps") ? body["reps"].i() : 0;
        w.weight = body.has("weight") ? body["weight"].d() : -1.0;
        w.duration = body.has("duration") ? body["duration"].i() : -1;
        w.notes = body.has("notes") ? body["notes"].s() : std::string("");
        w.session_id = body.has("session_id") ? body["session_id"].i() : 0;

        if (addWorkout(db, w))
            return crow::response(201, crow::json::wvalue{{"message", "Workout added successfully"}});
        else
            return makeError(500, "Failed to add workout");
    });

    // --- Get Workouts ---
    CROW_ROUTE(app, "/api/workouts").methods("GET"_method)([db](const crow::request& req)
    {
        // Read user_id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int user_id = std::stoi(user_id_str);

        const char* session_id_str = req.url_params.get("session_id");
        std::vector<Workout> workouts;

        if (session_id_str)
        {
            int session_id = std::stoi(session_id_str);
            workouts = getWorkoutsBySession(db, session_id);
        }
        else
        {
            const char* start_str = req.url_params.get("start");
            const char* end_str   = req.url_params.get("end");
            if (start_str && end_str)
                workouts = getUserWorkoutsByDate(db, user_id, start_str, end_str);
            else
                workouts = getUserWorkouts(db, user_id);
        }

        crow::json::wvalue res;
        crow::json::wvalue::list arr;

        for (const auto& w : workouts)
        {
            crow::json::wvalue item;
            item["id"] = w.id;
            item["user_id"] = w.user_id;
            item["date"] = w.date;
            item["type"] = w.type;
            item["sets"] = w.sets;
            item["reps"] = w.reps;
            item["weight"] = w.weight;
            item["duration"] = w.duration;
            item["notes"] = w.notes;
            item["session_id"] = w.session_id;

            arr.push_back(std::move(item));
        }

        res["workouts"] = std::move(arr);
        return crow::response(200, res);
    });

    // --- Update Workout ---
    CROW_ROUTE(app, "/api/workouts").methods("PUT"_method)([db](const crow::request& req)
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

        Workout w;
        w.id = body["id"].i();
        w.user_id = user_id;
        w.date = body["date"].s();
        w.type = body["type"].s();
        w.sets = body.has("sets") ? body["sets"].i() : 0;
        w.reps = body.has("reps") ? body["reps"].i() : 0;
        w.weight = body.has("weight") ? body["weight"].d() : -1.0;
        w.duration = body.has("duration") ? body["duration"].i() : -1;
        w.notes = body.has("notes") ? body["notes"].s() : std::string("");
        w.session_id = body.has("session_id") ? body["session_id"].i() : 0;

        if (updateWorkout(db, w))
            return crow::response(200, crow::json::wvalue{{"message", "Workout updated successfully"}});
        else
            return makeError(500, "Failed to update workout");
    });

    // --- Delete Workout ---
    CROW_ROUTE(app, "/api/workouts/<int>").methods("DELETE"_method)(
    [db](const crow::request& req, int workout_id)
    {
        // Read user_id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int user_id = std::stoi(user_id_str);

        if (deleteWorkout(db, workout_id, user_id))
            return crow::response(200, crow::json::wvalue{{"message", "Workout deleted successfully"}});
        else
            return makeError(404, "Workout not found or not owned by user");
    });
}
