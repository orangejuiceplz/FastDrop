cmake_minimum_required(VERSION 4.1)
project(FastDrop)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG master
)
FetchContent_MakeAvailable(crow)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 1.10.4
)
set(CPR_USE_SYSTEM_CURL OFF)
FetchContent_MakeAvailable(cpr)


file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(FastDrop ${SOURCES})


target_link_libraries(FastDrop PRIVATE
        Crow::Crow
        nlohmann_json::nlohmann_json
        cpr::cpr
)

target_include_directories(FastDrop PRIVATE include)