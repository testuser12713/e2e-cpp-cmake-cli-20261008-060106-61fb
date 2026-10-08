cmake_minimum_required(VERSION 3.20)

project(csvstats CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

# CLI-free core library: parse and statistics logic lives here and is unit
# tested directly through CTest without linking the executable (AC-10).
add_library(csvstats_core STATIC
    src/csv.cpp
    src/stats.cpp
    src/report.cpp
    src/cli.cpp
)
target_include_directories(csvstats_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(csvstats src/main.cpp)
target_link_libraries(csvstats PRIVATE csvstats_core)

enable_testing()
add_subdirectory(tests)
