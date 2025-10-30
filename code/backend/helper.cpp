#include "helper.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool validateMealData(const crow::json::rvalue& data, std::string& error) {
    if (!data.has("meal_type") || !data.has("meal_name") || !data.has("calories")) {
        error = "Missing required fields: meal_type, meal_name, and calories.";
        return false;
    }
    return true;
}
