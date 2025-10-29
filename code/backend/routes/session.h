#ifndef SESSION_H
#define SESSION_H

#include <sqlite3.h>
#include <crow.h>
#include <string>
#include <vector>

// Represents one session record
struct Session {
    int id;
    int user_id;
    std::string name;
    std::string date;
    std::string notes;
    int duration;
    std::string created_at;
    std::string updated_at;
};

// Function declarations
bool createSession(sqlite3* db, const Session& session);
std::vector<Session> getSessionsByUser(sqlite3* db, int user_id);
Session getSessionById(sqlite3* db, int session_id);
bool updateSession(sqlite3* db, const Session& session);
bool deleteSession(sqlite3* db, int session_id);

// routes
void setupSessionRoutes(crow::SimpleApp& app, sqlite3* db);

#endif
