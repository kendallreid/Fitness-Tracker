#include "crow.h"
#include "../db/workout_db.h"

void registerWorkoutRoutes(crow::SimpleApp& app, sqlite3* db)
{
    // Route to add a new workout
    CROW_ROUTE(app, "/api/workouts").methods("POST"_method)([db](const crow::request& req){

        // Parse JSON body
        auto body = crow::json::load(req.body);
        if (!body) {
            return makeError(400, "Invalid JSON");
        }

        // check if we have the required fields
        if (!body.has("user_id") || !body.has("date") || !body.has("type")) {
            return makeError(400, "Missing required fields");
        }

        // Extract workout details from JSON
        Workout w;
        w.user_id = body["user_id"].i();
        w.date = body["date"].s();
        w.type = body["type"].s();
        w.sets = body["sets"].i();
        
        if (body.has("reps") && body["reps"].t() == crow::json::type::Number)
        w.reps = body["reps"].i();

        if (body.has("weight") && body["weight"].t() == crow::json::type::Number)
            w.weight = body["weight"].d();
        else
            w.weight = -1.0; // mark as missing

        if (body.has("duration") && body["duration"].t() == crow::json::type::Number)
            w.duration = body["duration"].i();
        else
            w.duration = -1; // mark as missing

        w.notes = body["notes"].s();



    
        // add the workout to the database
        if (addWorkout(db, w)) {
            crow::json::wvalue res;
            res["message"] = "Workout added successfully";
            return crow::response(201, res);
        } else {
            return makeError(500, "Failed to add workout");
        }

    });

    // Route to get workouts
    CROW_ROUTE(app, "/api/workouts").methods("GET"_method)([db](const crow::request& req){

        // check to make sure how we retrieve the workouts
        auto user_id_str = req.url_params.get("user_id");
        if (!user_id_str) {
            return crow::response(400, "Missing user_id parameter");
        }

        int user_id;
        try {
            user_id = std::stoi(user_id_str);
        } catch (...) {
            return crow::response(400, "Invalid user_id parameter");
        }

        // try to get optional start and end date parameters
        auto start_str = req.url_params.get("start");
        auto end_str = req.url_params.get("end");

        std::vector<Workout> workouts;

        // if we have both start and end date parameters, filter workouts by date range
        if (start_str && end_str) {
            workouts = getUserWorkoutsByDate(db, user_id, start_str, end_str);
        }

        // if we don't have both start and end date parameters, get all workouts for the user
        else {
            workouts = getUserWorkouts(db, user_id);
        }

        crow::json::wvalue res;
        crow::json::wvalue::list arr;


        // set JSON response with workout details
        for (const auto& w : workouts) {
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

            arr.push_back(std::move(item));
        }

        res["workouts"] = std::move(arr);
        return crow::response(200, res);

    });

    // Route to update an existing workout
    CROW_ROUTE(app, "/api/workouts").methods("PUT"_method)([db](const crow::request& req){
        // Parse JSON body
        auto body = crow::json::load(req.body);
        if (!body) {
            return makeError(400, "Invalid JSON");
        }

        // check if we have the required fields
        if (!body.has("id") || !body.has("user_id") || !body.has("date") || !body.has("type")) {
            return makeError(400, "Missing required fields");
        }

        // Extract workout details from JSON
        Workout w;
        w.id = body["id"].i();
        w.user_id = body["user_id"].i();
        w.date = body["date"].s();
        w.type = body["type"].s();
        w.sets = body["sets"].i();
        w.reps = body["reps"].i();
        w.weight = body["weight"].d();
        w.duration = body["duration"].i();
        w.notes = body["notes"].s();

        // update the workout in the database
        if (updateWorkout(db, w)) {
            crow::json::wvalue res;
            res["message"] = "Workout updated successfully";
            return crow::response(200, res);
        } else {
            return makeError(500, "Failed to update workout");
        }
    });

    // Route to delete a workout
    CROW_ROUTE(app, "/api/workouts/<int>").methods("DELETE"_method)(
    [db](const crow::request& req, int workout_id)
    {
        // try to get user_id parameter
        auto user_id_str = req.url_params.get("user_id");
        if (!user_id_str)
            return makeError(400, "Missing user_id parameter");

        int user_id = std::stoi(user_id_str);

        // delete the workout
        if (deleteWorkout(db, workout_id, user_id)) {
            crow::json::wvalue res;
            res["message"] = "Workout deleted successfully";
            return crow::response(200, res);
        } else {
            return makeError(404, "Workout not found or not owned by user");
        }
    });
}
