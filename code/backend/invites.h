#pragma once
#include <string>
#include <optional>
#include <vector>
#include <sqlite3.h>
#include <crow.h>

struct FriendRequest {
    int id;
    int senderId;
    int receiverId;
    std::string status;
    std::string createdAt;
};    

struct Friendship {
    int userId1;
    int userId2;
    std::string createdAt;
};   

// Check if a user exists
bool userExists(sqlite3* db, int userId);

// Check if two users are already friends
bool friendshipExists(sqlite3* db, int userId1, int userId2);

// Find a pending request between two users (either direction)
std::optional<FriendRequest> findPendingFriendRequest(sqlite3* db, int userA, int userB);

// Insert a pending friend request and return its id
int insertFriendRequest(sqlite3* db, int senderId, int receiverId);

// Get all incoming (to me) pending requests
std::vector<FriendRequest> getIncomingRequests(sqlite3* db, int userId);

// Get all outgoing (from me) pending requests
std::vector<FriendRequest> getOutgoingRequests(sqlite3* db, int userId);

// Get all friendships for a user
std::vector<Friendship> getFriendships(sqlite3* db, int userId);

// Update request status (accept, reject, cancel)
bool updateFriendRequestStatus(sqlite3* db, int requestId, const std::string& newStatus);

// Create friendship (called on accept)
bool createFriendship(sqlite3* db, int userId1, int userId2);

// Get a request by id (for responding)
std::optional<FriendRequest> getFriendRequestById(sqlite3* db, int requestId);

// to get usernames by id
std::optional<std::string> getUsernameById(sqlite3* db, int userId);

void setupInviteRoutes(crow::SimpleApp& app, sqlite3* db);

std::string computeFriendStatus(sqlite3* db, int userId1, int userId2);