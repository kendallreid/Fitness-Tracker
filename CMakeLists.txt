cmake_minimum_required(VERSION 3.10)
project(FitnessApp)

set(CMAKE_CXX_STANDARD 17)

# Add Crow from external/ (uses its own CMakeLists.txt)
add_subdirectory(external/crow)

# Collect your backend source files
file(GLOB_RECURSE SRC_FILES
    code/backend/*.cpp
)

# Define your app
add_executable(fitness ${SRC_FILES})

# Link dependencies
find_package(Threads REQUIRED)
target_link_libraries(fitness
    PRIVATE
        Crow::Crow
        sqlite3
        Threads::Threads
)
