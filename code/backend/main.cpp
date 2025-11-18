#include <crow.h>
#include "LogIn.h"
#include <sodium.h>
#include "routes/register.h"
#include "goalTracker.h"
#include "db/schema.h"
#include <iostream>
#include "routes/exercise.h"
#include "routes/session.h"
#include "leaderboard.h"

#include "routes/calorie_tracker.h"
using namespace std;

int main() {
    crow::SimpleApp fitnessApp;

    // Initialize SQLite database connection
    sqlite3* db;
    if (sqlite3_open("code/backend/fitness.db", &db)) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    int rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to enable foreign keys: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "PRAGMA foreign_keys;", -1, &stmt, nullptr);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "Foreign keys enabled: " << sqlite3_column_int(stmt, 0) << std::endl;
    }
    sqlite3_finalize(stmt);

    if (!createTables(db)) {
    cerr << "Failed to create tables" << endl;
    sqlite3_close(db);
    return 1;
    }

    // Initialize libsodium
    if (sodium_init() < 0) {
        std::cerr << "Failed to initialize libsodium" << std::endl;
        return 1; // don’t continue if init fails
    }

//LOGIN//
    // Serve Login page
    CROW_ROUTE(fitnessApp, "/auth/login")
    ([]{
        return serveFile("code/frontend/login.html", "text/html");
    });

    // Initialize login manager with DB path
    LogInManager loginManager;

    // Hook up login routes
    setupLoginRoutes(fitnessApp, loginManager, db);

//REGISTRATION//
    // Serve registration page
    CROW_ROUTE(fitnessApp, "/")
    ([]{
        return serveFile("code/frontend/UserRegistration.html", "text/html");
    });

    // Hook up register routes
    setupRegisterRoutes(fitnessApp, db);

//HOME PAGE//
    // Serve home page
    CROW_ROUTE(fitnessApp, "/home")
    ([]{
        return serveFile("code/frontend/HomePage.html", "text/html");
    });

//SESSIONS//
    // Serve sessions page
    CROW_ROUTE(fitnessApp, "/sessions")
    ([]{
        return serveFile("code/frontend/sessions.html", "text/html");
    });

    // Serve exercises page
    CROW_ROUTE(fitnessApp, "/exercises.html")
    ([]{
        return serveFile("code/frontend/exercise.html", "text/html");
    });

    // Hook up session/exercise routes
    setupSessionRoutes(fitnessApp, db);
    registerExerciseRoutes(fitnessApp, db);

//GOALS//
    // Serve goals page
    CROW_ROUTE(fitnessApp, "/goals-page.html")
    ([]{
        return serveFile("code/frontend/goalTracker.html", "text/html");
    });

    CROW_ROUTE(fitnessApp, "/completedGoals.html")
    ([]{
        return serveFile("code/frontend/completedGoals.html", "text/html");
    });
    
    // Hook up goal routes
    setupGoalRoutes(fitnessApp, db);


        // Start server
    setupCalorieTrackerRoutes(fitnessApp, db);

//LEADERBOARD//
    CROW_ROUTE(fitnessApp, "/leaderboard.html")
    ([]{
        return serveFile("code/frontend/leaderboard.html", "text/html");
    });
    setupLeaderboardRoutes(fitnessApp, db);

//

    // Start server
    fitnessApp.port(8080).multithreaded().run();

    sqlite3_close(db);

    
    return 0;
}
