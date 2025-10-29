#include "session.h"
#include "workout.h"
#include "helper.h"

bool createSession(sqlite3 *db, const Session &session)
{
    const char *sql = "INSERT INTO sessions (user_id, name, date, notes, duration) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, session.user_id);
    sqlite3_bind_text(stmt, 2, session.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, session.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, session.notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, session.duration);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (success) {
        CROW_LOG_INFO << "Session created successfully";
    } else {
        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
        std::cerr << "user_id=" << session.user_id
                  << " name=" << session.name
                  << " date=" << session.date
                  << " notes=" << session.notes
                  << " duration=" << session.duration
                  << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

std::vector<Session> getSessionsByUser(sqlite3* db, int user_id)
{
    std::vector<Session> sessions;

    sqlite3_stmt *stmt;

    const char *sql = "SELECT * FROM sessions WHERE user_id = ?";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return sessions;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Session session;
        session.id = sqlite3_column_int(stmt, 0);
        session.user_id = sqlite3_column_int(stmt, 1);
        session.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        session.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        session.notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        session.duration = sqlite3_column_int(stmt, 5);
        session.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        session.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        sessions.push_back(session);
    }

    sqlite3_finalize(stmt);
    return sessions;
}

Session getSessionById(sqlite3 *db, int session_id)
{
    Session session;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM sessions WHERE id = ?";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Session();
    }

    sqlite3_bind_int(stmt, 1, session_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        session.id = sqlite3_column_int(stmt, 0);
        session.user_id = sqlite3_column_int(stmt, 1);
        session.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        session.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        session.notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        session.duration = sqlite3_column_int(stmt, 5);
        session.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        session.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    }

    sqlite3_finalize(stmt);
    return session;
}

bool updateSession(sqlite3 *db, const Session &session)
{
    const char *sql = "UPDATE sessions SET name = ?, date = ?, notes = ?, duration = ? WHERE id = ?";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, session.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, session.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, session.notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, session.duration);
    sqlite3_bind_int(stmt, 5, session.id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool deleteSession(sqlite3 *db, int session_id)
{
    const char *sql = "DELETE FROM sessions WHERE id = ?";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, session_id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

void setupSessionRoutes(crow::SimpleApp &app, sqlite3 *db)
{
    // Create a session
    CROW_ROUTE(app, "/api/sessions/create").methods("POST"_method)([db](const crow::request &req)
    {
        std::cout << "Raw body: [" << req.body << "]" << std::endl;

        // Read user_id from cookie instead of body
        auto cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return crow::response{401, "Unauthorized: missing login cookie"};
        }

        int user_id = std::stoi(user_id_str);

        Session session;
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response{400, "Invalid JSON"};
        }

        session.user_id = user_id; // cookie-based user
        session.name = body["name"].s();
        session.date = body["date"].s();
        session.notes = body.has("notes") ? body["notes"].s() : std::string{};
        session.duration = body.has("duration") ? body["duration"].i() : 0;

        if (createSession(db, session)) {
            return crow::response{201, "Session created successfully"};
        } else {
            return crow::response{500, "Failed to create session"};
        }
    });

    // Get sessions for the logged-in user
    CROW_ROUTE(app, "/api/sessions/user").methods("GET"_method)([db](const crow::request &req)
    {
        CROW_LOG_INFO << "Hit /api/sessions/user";

        auto cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return crow::response{401, "Unauthorized: missing login cookie"};
        }

        int user_id = std::stoi(user_id_str);

        std::vector<Session> sessions = getSessionsByUser(db, user_id);
        if (sessions.empty()) {
            return crow::response{404, "No sessions found for user"};
        }

        crow::json::wvalue res;
        crow::json::wvalue::list arr;
        for (const auto& s : sessions) {
            crow::json::wvalue item;
            item["id"] = s.id;
            item["user_id"] = s.user_id;
            item["name"] = s.name;
            item["date"] = s.date;
            item["notes"] = s.notes;
            item["duration"] = s.duration;
            item["created_at"] = s.created_at;
            item["updated_at"] = s.updated_at;
            arr.push_back(item);
        }

        res["sessions"] = std::move(arr);
        return crow::response(200, res);
    });

    // Get a single session
    CROW_ROUTE(app, "/api/sessions/<int>").methods("GET"_method)([db](const crow::request &req, int session_id)
    {
        Session session = getSessionById(db, session_id);
        if (session.id == 0) {
            return crow::response{404, "Session not found"};
        }

        crow::json::wvalue res;
        res["id"] = session.id;
        res["user_id"] = session.user_id;
        res["name"] = session.name;
        res["date"] = session.date;
        res["notes"] = session.notes;
        res["duration"] = session.duration;
        res["created_at"] = session.created_at;
        res["updated_at"] = session.updated_at;

        return crow::response{200, res};
    });

    // Get workouts for a session
    CROW_ROUTE(app, "/api/sessions/<int>/workouts").methods("GET"_method)([db](int session_id)
    {
        auto workouts = getWorkoutsBySession(db, session_id);

        crow::json::wvalue res;
        crow::json::wvalue::list arr;

        for (const auto& w : workouts) {
            crow::json::wvalue item;
            item["id"] = w.id;
            item["user_id"] = w.user_id;
            item["session_id"] = w.session_id;
            item["date"] = w.date;
            item["type"] = w.type;
            item["sets"] = w.sets;
            item["reps"] = w.reps;
            item["weight"] = w.weight;
            item["duration"] = w.duration;
            item["notes"] = w.notes;
            arr.push_back(std::move(item));
        }

        res["workouts"] = std::move(arr);
        return crow::response(200, res);
    });

    // Update a session
    CROW_ROUTE(app, "/api/sessions/<int>").methods("PUT"_method)([db](const crow::request &req, int session_id)
    {
        Session session = getSessionById(db, session_id);
        if (session.id == 0) {
            return crow::response{404, "Session not found"};
        }

        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response{400, "Invalid JSON"};
        }

        if (body.has("name")) session.name = body["name"].s();
        if (body.has("date")) session.date = body["date"].s();
        if (body.has("notes")) session.notes = body["notes"].s();
        if (body.has("duration")) session.duration = body["duration"].i();

        if (updateSession(db, session)) {
            return crow::response{200, "Session updated successfully"};
        } else {
            return crow::response{500, "Failed to update session"};
        }
    });

    // Delete a session
    CROW_ROUTE(app, "/api/sessions/<int>").methods("DELETE"_method)([db](const crow::request &req, int session_id)
    {
        if (deleteSession(db, session_id)) {
            return crow::response{204, "Session deleted successfully"};
        } else {
            return crow::response{500, "Failed to delete session"};
        }
    });
}
