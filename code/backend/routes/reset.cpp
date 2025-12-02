#include "reset.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>
#include <sodium.h>
#include "hash.h"
#include "helper.h"
#include <iostream>


EmailConfig load_email_config_from_env()
{

    
    // load email config from environment variables
    const char* mode = std::getenv("EMAIL_MODE");
    const char* api_key = std::getenv("EMAIL_API_KEY");
    const char* domain = std::getenv("EMAIL_DOMAIN");
    const char* from = std::getenv("EMAIL_FROM_ADDRESS");

    return EmailConfig{mode ? mode : "mock", api_key ? api_key : "", domain ? domain : "", from ? from : ""};
}

bool get_user_id_by_email(sqlite3 *db, const std::string &email, int &out_user_id)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id FROM users WHERE email = ? LIMIT 1;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out_user_id = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return true;
    } else {
        sqlite3_finalize(stmt);
    }

    return false;
}

bool create_password_reset_token(sqlite3 *db, int user_id, int expiry_seconds, std::string &out_token)
{
    // - Generates a random token
    std::string token = generate_secure_token(32); // 32 bytes raw

    // - Stores it in password_reset_tokens
    std::string expires_at = time_plus_seconds_iso8601(expiry_seconds);
    sqlite3_stmt *stmt;
    const char *sql = R"(
        INSERT INTO password_reset_tokens (user_id, token, expires_at, used)
        VALUES (?, ?, ?, 0);
    )";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, token.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, expires_at.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    out_token = token;
    return true;
}

std::optional<PasswordResetToken> get_password_reset_token(sqlite3 *db, const std::string &token)
{
    sqlite3_stmt *stmt;
    const char *sql = R"(
        SELECT id, user_id, token, expires_at, used
        FROM password_reset_tokens
        WHERE token = ? LIMIT 1;
    )";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        PasswordResetToken prt;
        prt.id = sqlite3_column_int(stmt, 0);
        prt.user_id = sqlite3_column_int(stmt, 1);
        prt.token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        prt.expires_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        prt.used = sqlite3_column_int(stmt, 4) != 0;
        sqlite3_finalize(stmt);
        return prt;
    } else {
        sqlite3_finalize(stmt);
    }

    return std::nullopt;
}

bool mark_reset_token_used(sqlite3 *db, int token_id)
{
    // prepare statement
    sqlite3_stmt *stmt;
    const char *sql = R"(
        UPDATE password_reset_tokens
        SET used = 1
        WHERE id = ?;
    )";

    // execute the statement
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_int(stmt, 1, token_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool update_user_password_hash(sqlite3 *db, int user_id, const std::string &new_password_hash)
{
    // prepare statement
    sqlite3_stmt *stmt;
    const char *sql = R"(
        UPDATE users
        SET password_hash = ?
        WHERE id = ?;
    )";

    // execute the statement
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, new_password_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, user_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool delete_expired_or_used_reset_tokens(sqlite3 *db)
{
    // prepare statement
    sqlite3_stmt *stmt;
    const char *sql = R"(
        DELETE FROM password_reset_tokens
        WHERE used = 1 OR expires_at <= ?;
    )";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, current_time_iso8601().c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

std::string generate_secure_token(std::size_t num_bytes)
{
    // generatye cryptographically secure random bytes
    unsigned char buffer[num_bytes];
    randombytes_buf(buffer, num_bytes);

    // encode as hex
    std::string token;
    char hex[3];
    for (std::size_t i = 0; i < num_bytes; ++i) {
        std::snprintf(hex, sizeof(hex), "%02x", buffer[i]);
        token += hex;
    }

    return token;
}

std::string current_time_iso8601()
{
    // Get current time in UTC
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm *utc_tm = std::gmtime(&now_c);

    // format as ISO 8601
    std::ostringstream oss;
    oss << std::put_time(utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

std::string time_plus_seconds_iso8601(int seconds)
{
    // Get current time + seconds in UTC 
    auto now = std::chrono::system_clock::now();
    auto future_time = now + std::chrono::seconds(seconds);
    std::time_t future_c = std::chrono::system_clock::to_time_t(future_time);
    std::tm *utc_tm = std::gmtime(&future_c);

    // format as ISO 8601
    std::ostringstream oss;
    oss << std::put_time(utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

bool is_token_expired(const std::string &expires_at_iso)
{
    // parse expires_at_iso
    std::tm tm = {};
    std::istringstream ss(expires_at_iso);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
        return true; // consider invalid format as expired
    }

    std::time_t expires_time = timegm(&tm);
    std::time_t now = std::time(nullptr);
    return expires_time <= now;
}

bool send_reset_email(const EmailConfig &cfg, const std::string &to, const std::string &reset_url)
{
    // Make sure reset_url is a FULL URL, e.g.
    // "https://your-domain.com/reset-password?token=ABC123"
    // NOT just "/reset-password?token=ABC123"

    std::string subject = "Password Reset Request";

    // Plain text version (fallback)
    std::string body_text =
        "You requested a password reset. Click the link below to reset your password:\n\n" +
        reset_url + "\n\n" +
        "If you did not request this, please ignore this email.";

    // HTML version (forces clickability)
    std::string body_html =
    "<!DOCTYPE html>"
    "<html><body>"
    "<p>You requested a password reset.</p>"
    "<p>Click the link below to reset your password:</p>"
    "<p><a href=\"" + reset_url + "\">" + reset_url + "</a></p>"
    "<p>If you did not request this, please ignore this email.</p>"
    "</body></html>";


    if (cfg.mode == "mock") 
    {
        std::cout << "[MOCK EMAIL] To: " << to
                  << "\nSubject: " << subject
                  << "\n\n[TEXT VERSION]\n" << body_text
                  << "\n\n[HTML VERSION]\n" << body_html
                  << "\n";
        return true;
    } 
    else if (cfg.mode == "real") 
    {
        // pass both text and html to Mailgun
        return send_email_via_mailgun(cfg, to, subject, body_text, body_html);
    }

    return false;
}


bool send_email_via_mailgun(const EmailConfig& cfg,
                            const std::string& to,
                            const std::string& subject,
                            const std::string& body_text,
                            const std::string& body_html)
{
    if (cfg.api_key.empty() || cfg.domain.empty() || cfg.from.empty()) {
        std::cerr << "[Mailgun] Missing config:"
                  << " api_key=" << (cfg.api_key.empty() ? "EMPTY" : "SET")
                  << " domain="  << (cfg.domain.empty() ? "EMPTY" : cfg.domain)
                  << " from="    << (cfg.from.empty() ? "EMPTY" : cfg.from)
                  << "\n";
        return false;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[Mailgun] curl_easy_init() failed\n";
        return false;
    }

    bool ok = false;

    std::ostringstream url;
    url << "https://api.mailgun.net/v3/" << cfg.domain << "/messages";
    std::cerr << "[Mailgun] URL = " << url.str() << "\n";

    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERNAME, "api");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, cfg.api_key.c_str());

    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part = nullptr;

    // From
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "from");
    curl_mime_data(part, cfg.from.c_str(), CURL_ZERO_TERMINATED);

    // To
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "to");
    curl_mime_data(part, to.c_str(), CURL_ZERO_TERMINATED);

    // Subject
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "subject");
    curl_mime_data(part, subject.c_str(), CURL_ZERO_TERMINATED);

    // Plain text body
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "text");
    curl_mime_data(part, body_text.c_str(), CURL_ZERO_TERMINATED);

    // HTML body (this is what makes the “button” version clickable)
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "html");
    curl_mime_data(part, body_html.c_str(), CURL_ZERO_TERMINATED);

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    std::string response_body;
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
            auto* resp = static_cast<std::string*>(userdata);
            resp->append(ptr, size * nmemb);
            return size * nmemb;
        }
    );
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "[Mailgun] curl_easy_perform() failed: "
                  << curl_easy_strerror(res) << "\n";
    } else {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        std::cerr << "[Mailgun] HTTP " << http_code
                  << " response: " << response_body << "\n";

        if (http_code == 200 || http_code == 202) {
            ok = true;
        } else {
            std::cerr << "[Mailgun] Non-success HTTP code from Mailgun\n";
        }
    }

    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    return ok;
}




void setupPasswordResetRoutes(crow::SimpleApp &app, sqlite3 *db, const EmailConfig &email_cfg)
{
    // POST /auth/api/forgot-password
    CROW_ROUTE(app, "/auth/api/forgot-password").methods("POST"_method)(
        [db, email_cfg](const crow::request &req) {
            auto body = crow::json::load(req.body);
            if (!body || !body.has("email")) {
                return makeError(400, "Invalid JSON or missing email");
            }

            std::string email = body["email"].s();
            int userId;
            if (!get_user_id_by_email(db, email, userId)) {
                // To prevent email enumeration, respond with success even if email not found
                return crow::response(200, crow::json::wvalue{{"status", "success"}});
            }

            std::string token;
            if (!create_password_reset_token(db, userId, 3600, token)) { // 1 hour expiry
                return makeError(500, "Failed to create reset token");
            }

            std::string resetUrl = "http://localhost:8080/auth/new-password?token=" + token;
            if (!send_reset_email(email_cfg, email, resetUrl)) {
                return makeError(500, "Failed to send reset email");
            }

            return crow::response(200, crow::json::wvalue{{"status", "success"}});
        }
    );

    // GET auth/api/reset-password/validate?token=...
    CROW_ROUTE(app, "/auth/api/reset-password/validate").methods("GET"_method)(
        [db](const crow::request &req) {

            // get token from query param
            auto token = req.url_params.get("token");
            if (!token) {
                return makeError(400, "Missing token");
            }

            // lookup token
            auto prtOpt = get_password_reset_token(db, token);
            if (!prtOpt) {
                return makeError(400, "Invalid token");
            }

            auto &prt = *prtOpt;

            // check used
            if (prt.used) {
                return makeError(400, "Token already used");
            }

            // check expiry
            if (is_token_expired(prt.expires_at)) {
                return makeError(400, "Token expired");
            }

            return crow::response(200, crow::json::wvalue{{"status", "valid"}});
        }
    );


    // POST auth/api/reset-password
    CROW_ROUTE(app, "/auth/api/reset-password").methods("POST"_method)(
        [db](const crow::request &req) {

            // parse body
            auto body = crow::json::load(req.body);
            if (!body || !body.has("token") || !body.has("new_password")) {
                return makeError(400, "Invalid JSON or missing fields");
            }
            std::string token = body["token"].s();
            std::string newPassword = body["new_password"].s();

            // lookup token
            auto prtOpt = get_password_reset_token(db, token);
            if (!prtOpt) {
                return makeError(400, "Invalid token");
            }
            auto &prt = *prtOpt;

            // check used
            if (prt.used) {
                return makeError(400, "Token already used");
            }

            // check expiry
            if (is_token_expired(prt.expires_at)) {
                return makeError(400, "Token expired");
            }

            // hash new password
            std::string newPasswordHash = hashPassword(newPassword);
            if (!update_user_password_hash(db, prt.user_id, newPasswordHash)) {
                return makeError(500, "Failed to update password");
            }

            // mark token used
            if (!mark_reset_token_used(db, prt.id)) {
                return makeError(500, "Failed to mark token used");
            }

            return crow::response(200, crow::json::wvalue{{"status", "success"}});
        }
    );
}