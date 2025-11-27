#pragma once

#include <string>
#include <optional>
#include <sqlite3.h>
#include <crow.h>

struct EmailConfig {
    std::string mode;      // "mock" or "real"
    std::string api_key;   // Mailgun API key
    std::string domain;    // Mailgun domain, e.g. sandboxXXXX.mailgun.org
    std::string from;      // From address, e.g. "no-reply@fitnessapp.com"
};

EmailConfig load_email_config_from_env();

struct PasswordResetToken {
    int id;
    int user_id;
    std::string token;
    std::string expires_at;  // ISO 8601 string, e.g. "2025-11-26T18:30:00Z"
    bool used;
};


// Look up a user_id by email. Returns true and fills user_id if found.
bool get_user_id_by_email(sqlite3* db, const std::string& email, int& out_user_id);

// Create a password reset token for the given user_id, valid for expiry_seconds.
bool create_password_reset_token(sqlite3* db, int user_id, int expiry_seconds, std::string& out_token);

// Fetch full token info by its token string.
// Returns std::nullopt if not found.
std::optional<PasswordResetToken> get_password_reset_token(sqlite3* db, const std::string& token);

// Mark a reset token as used = 1.
bool mark_reset_token_used(sqlite3* db, int token_id);

// Update the user's password hash in the users table.
// (Assumes you already hashed the password before calling this.)
bool update_user_password_hash(sqlite3* db, int user_id, const std::string& new_password_hash);

// (Optional but nice) Delete tokens that are expired or already used.
bool delete_expired_or_used_reset_tokens(sqlite3* db);


// Generate a cryptographically secure random token, encoded as hex or base64.
// num_bytes is the raw random bytes before encoding, e.g. 32.
std::string generate_secure_token(std::size_t num_bytes);

// Get current PST time as ISO 8601 string.
std::string current_time_iso8601();

// Get (current time + seconds) as ISO 8601 string.
std::string time_plus_seconds_iso8601(int seconds);

// Compare current time with expires_at ISO string to see if token is expired.
bool is_token_expired(const std::string& expires_at_iso);

bool send_reset_email(const EmailConfig& cfg, const std::string& to, const std::string& reset_url);

bool send_email_via_mailgun(const EmailConfig& cfg, const std::string& to, const std::string& subject, const std::string& body_text);

void setupPasswordResetRoutes(crow::SimpleApp& app, sqlite3* db, const EmailConfig& email_cfg);