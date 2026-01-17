//
// Created by sakuya on 1/17/26.
//

#pragma once
#include "crow.h"
#include "crow/middlewares/cors.h"

namespace fastdrop {
    void register_routes(crow::App<crow::CORSHandler>& app);
}