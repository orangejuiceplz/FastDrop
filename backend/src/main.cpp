//
// Created by orangejuiceplz on 1/17/26.
//

#include <iostream>
#include <filesystem>
#include <thread>
#include <atomic>
#define CROW_ENABLE_CORS
#include "crow.h"
#include "routes.hpp"
#include "config.hpp"
#include "file_storage.hpp"

namespace fs = std::filesystem;

std::atomic<bool> running{true};

void cleanup_thread() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        if (running) {
            fastdrop::FileStorage::instance().cleanup_expired();
        }
    }
}

int main() {
    crow::App<crow::CORSHandler> app;

    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("http://localhost:5173")
        .methods("GET"_method, "POST"_method, "OPTIONS"_method)
        .headers("Content-Type");

    fs::create_directories(fastdrop::TEMP_DIR);

    std::thread cleaner(cleanup_thread);

    fastdrop::register_routes(app);

    std::cout << " FastDrop running on http:localhost:1337" << std::endl;
    app.port(1337).multithreaded().run();

    running = false;
    cleaner.join();

    return 0;
}
