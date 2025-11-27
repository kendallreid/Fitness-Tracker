cmake_minimum_required(VERSION 3.10)
project(FitnessApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------
# Packages from vcpkg
# ----------------------------------------------------------------------
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

# ----------------------------------------------------------------------
# Source files
# ----------------------------------------------------------------------
file(GLOB_RECURSE SRC_FILES
    code/backend/*.cpp
)

add_executable(fitness ${SRC_FILES})

# ----------------------------------------------------------------------
# Include directories
# Crow + libsodium + sqlite3 headers all live under vcpkg include
# ----------------------------------------------------------------------
target_include_directories(fitness PRIVATE
    ${CMAKE_SOURCE_DIR}/code/backend
    ${CMAKE_SOURCE_DIR}/code/backend/routes
    ${VCPKG_INSTALLED_DIR}/x64-linux/include
)

# ----------------------------------------------------------------------
# Link Libraries
# libsodium is a plain library, so we link manually
# ----------------------------------------------------------------------
target_link_libraries(fitness PRIVATE
    SQLite::SQLite3
    Threads::Threads
    CURL::libcurl
    ${VCPKG_INSTALLED_DIR}/x64-linux/lib/libsodium.a
)
