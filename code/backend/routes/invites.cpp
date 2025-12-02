#include "../invites.h"
#include <iostream>
#include "helper.h"
#include "invites.h"

using namespace std;

bool userExists(sqlite3 *db, int userId)
{
    // Prepare SQL statement to check for user existence
    const char *sql = "SELECT 1 FROM users WHERE id = ? LIMIT 1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    // Bind userId parameter and execute
    sqlite3_bind_int(stmt, 1, userId);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

bool friendshipExists(sqlite3 *db, int userId1, int userId2)
{
    // Ensure userId1 is less than userId2 for symmetric representation
    if (userId1 > userId2) std::swap(userId1, userId2);

    // Prepare SQL statement to check for friendship existence
    const char *sql = "SELECT 1 FROM friendships WHERE user_id1 = ? AND user_id2 = ? LIMIT 1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    // Bind parameters and execute
    sqlite3_bind_int(stmt, 1, userId1);
    sqlite3_bind_int(stmt, 2, userId2);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    return exists;
}

std::optional<FriendRequest> findPendingFriendRequest(sqlite3 *db, int userA, int userB)
{
    // Prepare SQL statement to find pending friend request between two users
    const char *sql = R"(
        SELECT id, sender_id, receiver_id, status, created_at
        FROM friend_requests
        WHERE ((sender_id = ? AND receiver_id = ?) OR (sender_id = ? AND receiver_id = ?))
          AND status = 'pending'
        LIMIT 1;
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    // Bind parameters and execute
    sqlite3_bind_int(stmt, 1, userA);
    sqlite3_bind_int(stmt, 2, userB);
    sqlite3_bind_int(stmt, 3, userB);
    sqlite3_bind_int(stmt, 4, userA);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        FriendRequest fr;
        fr.id = sqlite3_column_int(stmt, 0);
        fr.senderId = sqlite3_column_int(stmt, 1);
        fr.receiverId = sqlite3_column_int(stmt, 2);
        fr.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        fr.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        sqlite3_finalize(stmt);
        return fr;
    } else {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
}

int insertFriendRequest(sqlite3 *db, int senderId, int receiverId)
{
    // Prepare SQL statement to insert a new friend request
    const char *sql = R"(
        INSERT INTO friend_requests (sender_id, receiver_id, status)
        VALUES (?, ?, 'pending');
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare statement in insertFriendRequest: " << sqlite3_errmsg(db) << endl;
        return 0;
    }

    // Bind parameters and execute
    sqlite3_bind_int(stmt, 1, senderId);
    sqlite3_bind_int(stmt, 2, receiverId);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Failed to execute statement in insertFriendRequest: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return 0;
    }

    int lastId = (int)sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return lastId;
}

std::vector<FriendRequest> getIncomingRequests(sqlite3 *db, int userId)
{
    // Prepare SQL statement to get incoming pending friend requests
    const char *sql = R"(
        SELECT id, sender_id, receiver_id, status, created_at
        FROM friend_requests
        WHERE receiver_id = ? AND status = 'pending';
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::vector<FriendRequest>();
    }

    // Bind parameters and execute
    sqlite3_bind_int(stmt, 1, userId);
    std::vector<FriendRequest> requests;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FriendRequest fr;
        fr.id = sqlite3_column_int(stmt, 0);
        fr.senderId = sqlite3_column_int(stmt, 1);
        fr.receiverId = sqlite3_column_int(stmt, 2);
        fr.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        fr.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        requests.push_back(fr);
    }
    sqlite3_finalize(stmt);
    return requests;
}

std::vector<FriendRequest> getOutgoingRequests(sqlite3 *db, int userId)
{
    // Prepare SQL statement to get outgoing pending friend requests
    const char *sql = R"(
        SELECT id, sender_id, receiver_id, status, created_at
        FROM friend_requests
        WHERE sender_id = ? AND status = 'pending';
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::vector<FriendRequest>();
    }

    // Bind parameters and execute
    sqlite3_bind_int(stmt, 1, userId);
    std::vector<FriendRequest> requests;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FriendRequest fr;
        fr.id = sqlite3_column_int(stmt, 0);
        fr.senderId = sqlite3_column_int(stmt, 1);
        fr.receiverId = sqlite3_column_int(stmt, 2);
        fr.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        fr.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        requests.push_back(fr);
    }
    sqlite3_finalize(stmt);
    return requests;
}

std::vector<Friendship> getFriendships(sqlite3 *db, int userId)
{
    // Prepare SQL statement to get all friendships for a user
    const char *sql = R"(
        SELECT user_id1, user_id2, created_at
        FROM friendships
        WHERE user_id1 = ? OR user_id2 = ?;
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::vector<Friendship>();
    }
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, userId);
    std::vector<Friendship> friendships;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Friendship fs;
        fs.userId1 = sqlite3_column_int(stmt, 0);
        fs.userId2 = sqlite3_column_int(stmt, 1);
        fs.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        friendships.push_back(fs);
    }
    sqlite3_finalize(stmt);
    return friendships;
}

bool updateFriendRequestStatus(sqlite3 *db, int requestId, const std::string &newStatus)
{
    // Prepare SQL statement to update friend request status
    const char *sql = R"(
        UPDATE friend_requests
        SET status = ?
        WHERE id = ?;
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, newStatus.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, requestId);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool createFriendship(sqlite3 *db, int userId1, int userId2)
{
    // Ensure userId1 is less than userId2 for symmetric representation
    if (userId1 > userId2) std::swap(userId1, userId2);

    // Prepare SQL statement to create a new friendship
    const char *sql = R"(
        INSERT INTO friendships (user_id1, user_id2)
        VALUES (?, ?);
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    // Bind parameters and execute
    sqlite3_bind_int(stmt, 1, userId1);
    sqlite3_bind_int(stmt, 2, userId2);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    
    return true;
}

std::optional<std::string> getUsernameById(sqlite3 *db, int userId)
{
    // Prepare SQL statement to get username by user ID
    const char *sql = "SELECT username FROM users WHERE id = ? LIMIT 1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::optional<std::string>();
    }

    // Bind userId parameter and execute
    sqlite3_bind_int(stmt, 1, userId);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::optional<std::string>();
    }

    // Extract username from the result
    std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    sqlite3_finalize(stmt);
    return std::optional<std::string>(username);
}

void setupInviteRoutes(crow::SimpleApp &app, sqlite3 *db)
{
    // send friend request
    CROW_ROUTE(app, "/api/friend-requests").methods("POST"_method)([db](const crow::request &req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("receiver_id")) {
            return makeError(400, "Invalid JSON or missing receiver_id");
        }

        // Read user_id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int senderId = std::stoi(user_id_str);
        int receiverId = body["receiver_id"].i();

        // Check if users exist
        if (!userExists(db, senderId) || !userExists(db, receiverId)) {
            return makeError(404, "User not found");
        }

        // Check if they are already friends
        if (friendshipExists(db, senderId, receiverId)) {
            return makeError(400, "You are already friends");
        }

        // Check for existing pending request
        auto existingRequest = findPendingFriendRequest(db, senderId, receiverId);
        if (existingRequest) {
            return makeError(400, "A pending friend request already exists between these users");
        }

        // Insert new friend request
        int requestId = insertFriendRequest(db, senderId, receiverId);
        if (requestId == 0) {
            return makeError(500, "Failed to create friend request");
        }

        crow::json::wvalue res;
        res["status"] = "success";
        res["request_id"] = requestId;
        return crow::response(201, res);
    });

    // get incoming friend requests
    CROW_ROUTE(app, "/api/friend-requests/incoming").methods("GET"_method)([db](const crow::request &req) {
        // Read user_id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int userId = std::stoi(user_id_str);

        auto requests = getIncomingRequests(db, userId);

        crow::json::wvalue res;
        crow::json::wvalue::list arr;

        for (const auto &fr : requests) {
            crow::json::wvalue frJson;
            frJson["id"] = fr.id;
            frJson["sender_id"] = fr.senderId;
            frJson["receiver_id"] = fr.receiverId;
            frJson["sender_username"] = getUsernameById(db, fr.senderId).value_or("");
            frJson["status"] = fr.status;
            frJson["created_at"] = fr.createdAt;
            arr.push_back(frJson);
        }

        res["requests"] = std::move(arr);

        return crow::response(200, res);
    });

    // respond to friend request
    CROW_ROUTE(app, "/api/friend-requests/<int>").methods("POST"_method)([db](const crow::request &req, int requestId) {

        // extract request id from json body
        auto body = crow::json::load(req.body);
        if (!body || !body.has("action")) {
            return makeError(400, "Invalid JSON or missing action");    
        }

        std::string action = body["action"].s();

        // verify current user is the receiver of the request
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int currentUserId = std::stoi(user_id_str);

        // verify status is pending and get sender/receiver ids from DB
        const char *sql = R"(
            SELECT sender_id, receiver_id, status
            FROM friend_requests    
            WHERE id = ?;
        )";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return makeError(500, "Database error");
        }
        sqlite3_bind_int(stmt, 1, requestId);
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return makeError(404, "Friend request not found");
        }
        int senderId = sqlite3_column_int(stmt, 0);
        int receiverId = sqlite3_column_int(stmt, 1);
        std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        sqlite3_finalize(stmt);

        if (receiverId != currentUserId) {
            return makeError(403, "You are not authorized to respond to this friend request");
        }
        if (status != "pending") {
            return makeError(400, "This friend request has already been responded to");
        }

        // obtain new status
        std::string newStatus;
        if (action == "accept") {
            newStatus = "accepted";
            // create friendship
            if (!createFriendship(db, senderId, receiverId)) {
                return makeError(500, "Failed to create friendship");
            }
        } 
        else if (action == "reject") {
            newStatus = "rejected";
        } 
        else {
            return makeError(400, "Invalid action. Must be 'accept' or 'reject'");
        }

        // update friend request status in DB
        const char *updateSql = R"(
            UPDATE friend_requests
            SET status = ?
            WHERE id = ?;
        )";
        sqlite3_stmt *updateStmt;
        if (sqlite3_prepare_v2(db, updateSql, -1, &updateStmt, nullptr) != SQLITE_OK) {
            return makeError(500, "Database error");
        }
        sqlite3_bind_text(updateStmt, 1, newStatus.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(updateStmt, 2, requestId);
        if (sqlite3_step(updateStmt) != SQLITE_DONE) {
            sqlite3_finalize(updateStmt);
            return makeError(500, "Failed to update friend request status");
        }
        sqlite3_finalize(updateStmt);

        crow::json::wvalue res;
        res["status"] = "success";
        res["new_status"] = newStatus;
        return crow::response(200, res);
    });

    // get all friendships
    CROW_ROUTE(app, "/api/friends").methods("GET"_method)([db](const crow::request &req) {
        // Read user_id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int userId = std::stoi(user_id_str);

        auto friendships = getFriendships(db, userId);

        crow::json::wvalue res;
        crow::json::wvalue::list arr;

        for (const auto &f : friendships) {

            // Determine the friend (the *other* user)
            int friendId = (f.userId1 == userId) ? f.userId2 : f.userId1;

            // Get username once
            auto usernameOpt = getUsernameById(db, friendId);
            std::string username = usernameOpt.has_value() ? *usernameOpt : "";

            crow::json::wvalue item;
            item["user_id"] = friendId;
            item["username"] = username;
            item["created_at"] = f.createdAt;

            arr.push_back(std::move(item));
        }

        res["friendships"] = std::move(arr);
        return crow::response(200, res);
    });

    // find friends by username across the platform
    CROW_ROUTE(app, "/api/friends/search").methods("GET"_method)([db](const crow::request &req) {
        // Extract 'username' query parameter
        auto urlParams = req.url_params;
        if (!urlParams.get("username")) {
            return makeError(400, "Missing username query parameter");
        }
        std::string searchUsername = urlParams.get("username");

        // obtiain current user id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int userId = std::stoi(user_id_str);

        // Prepare SQL statement to search users by username
        const char *sql = R"(
            SELECT id, username
            FROM users
            WHERE username LIKE ? LIMIT 10;
        )";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return makeError(500, "Database error");
        }

        std::string likePattern = "%" + searchUsername + "%";
        sqlite3_bind_text(stmt, 1, likePattern.c_str(), -1, SQLITE_TRANSIENT);

        crow::json::wvalue res;
        crow::json::wvalue::list arr;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int foundId = sqlite3_column_int(stmt, 0);
            const char *uname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

            if (foundId == userId) continue; // Skip myself

            // Determine friend status
            std::string status = computeFriendStatus(db, userId, foundId);

            crow::json::wvalue item;
            item["user_id"] = foundId;
            item["username"] = uname ? uname : "";
            item["friend_status"] = status;

            arr.push_back(std::move(item));
        }
        sqlite3_finalize(stmt);

        res["results"] = std::move(arr);
        return crow::response(200, res);

    });

    // cancel outgoing pending invite
    CROW_ROUTE(app, "/api/invites/cancel/<int>").methods("POST"_method)([db](const crow::request &req, int inviteId) {

        // get user id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int userId = std::stoi(user_id_str);

        // prepare sql statement to find the invite
        const char *sql = R"(
            SELECT sender_id, receiver_id, status
            FROM friend_requests
            WHERE id = ?;
        )";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return makeError(500, "Database error");
        }

        sqlite3_bind_int(stmt, 1, inviteId);
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return makeError(404, "Invite not found");
        }

        int senderId = sqlite3_column_int(stmt, 0);
        int receiverId = sqlite3_column_int(stmt, 1);
        const char* statusText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string status = statusText ? statusText : "";

        sqlite3_finalize(stmt);

        if (senderId != userId) {
            return makeError(403, "You are not authorized to cancel this invite");
        }
        if (status != "pending") {
            return makeError(400, "This invite has already been responded to");
        }

        // delete the invite (set its status to cancelled)
        if (!updateFriendRequestStatus(db, inviteId, "cancelled")) {
            return makeError(500, "Failed to cancel invite");
        }
        
        crow::json::wvalue res;
        res["invite_id"] = inviteId;
        res["status"] = "cancelled";
        return crow::response(200, res);
    });

    // remove friend
    CROW_ROUTE(app, "/api/friends/remove/<int>").methods("POST"_method)([db](const crow::request &req, int friendId) {
        // get user id from cookie
        std::string cookieHeader = req.get_header_value("Cookie");
        std::string user_id_str = getCookieValue(cookieHeader, "user_id");
        if (user_id_str.empty()) {
            return makeError(401, "Unauthorized: missing login cookie");
        }
        int userId = std::stoi(user_id_str);

        // check if they are friends
        if (!friendshipExists(db, userId, friendId)) {
            return makeError(400, "You are not friends with this user");
        }

        // delete friendship
        const char *sql = R"(
            DELETE FROM friendships
            WHERE user_id1 = ? AND user_id2 = ?;
        )";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return makeError(500, "Database error");
        }
        sqlite3_bind_int(stmt, 1, std::min(userId, friendId));
        sqlite3_bind_int(stmt, 2, std::max(userId, friendId));

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return makeError(500, "Failed to remove friend");
        }
        sqlite3_finalize(stmt);

        crow::json::wvalue res;
        res["status"] = "success";
        return crow::response(200, res);
    });

}

std::string computeFriendStatus(sqlite3 *db, int userId1, int userId2)
{
    // Check if they are already friends
    if (friendshipExists(db, userId1, userId2)) {
        return "friends";
    }

    // Check for existing pending friend request
    auto existingRequest = findPendingFriendRequest(db, userId1, userId2);
    if (existingRequest) {
        if (existingRequest->senderId == userId1) {
            return "request_sent";
        } else {
            return "request_received";
        }
    }

    return "none";
}
