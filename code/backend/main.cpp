#include <crow.h>
#include "LogIn.h"

void setupLoginRoutes(crow::SimpleApp& app, LogInManager& loginManager);

int main() {
    crow::SimpleApp fitnessApp;

    // Initialize login manager with DB path
    LogInManager loginManager("backend/fitness.db");

    // Hook up login routes
    setupLoginRoutes(fitnessApp, loginManager);

    // Start server
    fitnessApp.port(18080).multithreaded().run();
    return 0;
}
