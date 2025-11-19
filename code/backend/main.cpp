#include <crow.h>
#include "LogIn.h"
#include <sodium.h>
#include "routes/register.h"
#include "goalTracker.h"
#include "db/schema.h"
#include <iostream>
#include "routes/workout.h"
#include "routes/session.h"
#include "routes/sleep_tracker.h"
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

    // Turn on foreign key support
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

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

    // Serve workouts page
    CROW_ROUTE(fitnessApp, "/workouts.html")
    ([]{
        return serveFile("code/frontend/workouts.html", "text/html");
    });

    // Hook up session/workout routes
    setupSessionRoutes(fitnessApp, db);
    registerWorkoutRoutes(fitnessApp, db);

//GOALS//
    // Serve goals page
    CROW_ROUTE(fitnessApp, "/goals-page.html")
    ([]{
        return serveFile("code/frontend/goalTracker.html", "text/html");
    });

    // Hook up goal routes
    setupGoalRoutes(fitnessApp, db);


        // Start server
    setupCalorieTrackerRoutes(fitnessApp, db);

     // Start sleep tracker server
    setupSleepTrackerRoutes(fitnessApp, db);

    // Start server
    fitnessApp.port(8080).multithreaded().run();
    sqlite3_close(db);

    
    return 0;
}
