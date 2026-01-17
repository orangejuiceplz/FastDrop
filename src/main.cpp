//
// Created by orangejuiceplz on 1/17/26.
//

#include <iostream>
#include <filesystem>
#include "crow.h"
#include "routes.hpp"
#include "config.hpp"

namespace fs = std::filesystem;

int main() {
    crow::SimpleApp app;

    fs::create_directories(fastdrop::TEMP_DIR);

    fastdrop::register_routes(app);

    std::cout << " FastDrop running on http://localhost:18080" << std::endl;
    app.port(18080).multithreaded().run();
    return 0;
}
