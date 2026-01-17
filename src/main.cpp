//
// Created by orangejuiceplz on 1/17/26.
//

#include <iostream>
#include <filesystem>
#include <thread>
#include <atomic>
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
    crow::SimpleApp app;

    fs::create_directories(fastdrop::TEMP_DIR);

    std::thread cleaner(cleanup_thread);

    fastdrop::register_routes(app);

    std::cout << " FastDrop running on http:localhost:5173" << std::endl;
    app.port(5173).multithreaded().run();

    running = false;
    cleaner.join();

    return 0;
}
