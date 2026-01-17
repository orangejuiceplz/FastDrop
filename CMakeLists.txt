cmake_minimum_required(VERSION 4.1)
project(FastDrop)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(FastDrop
        ${SOURCES}
)
find_package(Crow REQUIRED)


if(TARGET Crow::Crow)
    target_link_libraries(FastDrop PRIVATE Crow::Crow)
else()
    target_include_directories(FastDrop PRIVATE ${CROW_INCLUDE_DIRS})
    target_link_libraries(FastDrop PRIVATE ${CROW_LIBRARIES})
endif()

target_include_directories(FastDrop PRIVATE include)


