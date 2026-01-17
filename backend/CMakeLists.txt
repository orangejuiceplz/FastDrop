cmake_minimum_required(VERSION 4.1) # This is valid in 2026
project(FastDrop)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_POLICY_VERSION_MINIMUM "3.5")

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

FetchContent_Declare(
        dotenv
        GIT_REPOSITORY https://github.com/laserpants/dotenv-cpp.git
        GIT_TAG master
)
FetchContent_MakeAvailable(dotenv)

FetchContent_Declare(
        qrcodegen
        GIT_REPOSITORY https://github.com/nayuki/QR-Code-generator.git
        GIT_TAG master
)
FetchContent_MakeAvailable(qrcodegen)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(FastDrop ${SOURCES}
        include/file_storage.hpp
        include/code_gen.hpp
        include/routes.hpp
        include/config.hpp
        include/ai_service.hpp
        include/session_manager.hpp
        src/code_gen.cpp
        src/routes.cpp
        src/file_storage.cpp
        src/ai_service.cpp
        src/session_manager.cpp
        ${qrcodegen_SOURCE_DIR}/cpp/qrcodegen.cpp)

target_link_libraries(FastDrop PRIVATE
        Crow::Crow
        nlohmann_json::nlohmann_json
        cpr::cpr
)

target_include_directories(FastDrop PRIVATE 
        include
        ${dotenv_SOURCE_DIR}/include
        ${qrcodegen_SOURCE_DIR}/cpp
)