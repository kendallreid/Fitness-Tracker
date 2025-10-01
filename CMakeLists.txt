cmake_minimum_required(VERSION 3.10)
project(FitnessApp)

set(CMAKE_CXX_STANDARD 17)

# Must be BEFORE add_subdirectory
option(CROW_USE_BOOST "Use Boost.Asio instead of standalone Asio" ON)

# Add Crow from external/ (uses its own CMakeLists.txt)
add_subdirectory(external/crow)

# Find libsodium
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

# Collect backend source files
file(GLOB_RECURSE SRC_FILES
    code/backend/*.cpp
)

# Define your app target
add_executable(fitness ${SRC_FILES})

# Make sure your target can see libsodium headers
target_include_directories(fitness PRIVATE ${SODIUM_INCLUDE_DIRS})

# Link dependencies
find_package(Threads REQUIRED)
target_link_libraries(fitness
    PRIVATE
        ${SODIUM_LIBRARIES}
        Crow::Crow
        sqlite3
        Threads::Threads
)
