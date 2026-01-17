//
// Created by orangejuiceplz on 1/17/26.
//

#include <iostream>
#include <filesystem>
#include <thread>
#include <atomic>
#include <cstdlib>
#include "laserpants/dotenv/dotenv.h"
#include "crow.h"
#include "crow/middlewares/cors.h"
#include "routes.hpp"
#include "config.hpp"
#include "file_storage.hpp"
#include "ai_service.hpp"

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
    std::vector<std::string> env_paths = {".env", "../.env", "../../.env"};
    for (const auto& path : env_paths) {
        if (fs::exists(path)) {
            dotenv::init(path.c_str());
            std::cout << " Loaded .env from: " << path << std::endl;
            break;
        }
    }

    crow::App<crow::CORSHandler> app;

    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("http://localhost:5173")
        .methods("GET"_method, "POST"_method, "OPTIONS"_method)
        .headers("Content-Type");

    fs::create_directories(fastdrop::TEMP_DIR);

    const char* gemini_key = std::getenv("GEMINI_API_KEY");
    if (gemini_key && strlen(gemini_key) > 0) {
        fastdrop::AIService::set_api_key(gemini_key);
        std::cout << " AI summarization enabled (Gemini)" << std::endl;
    } else {
        std::cout << " AI summarization disabled (add GEMINI_API_KEY to .env)" << std::endl;
    }

    std::thread cleaner(cleanup_thread);

    fastdrop::register_routes(app);

    std::cout << " FastDrop running on http://localhost:1337" << std::endl;
    app.port(1337).multithreaded().run();

    running = false;
    cleaner.join();

    return 0;
}
