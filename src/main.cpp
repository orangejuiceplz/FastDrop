//
// Created by orangejuiceplz on 1/17/26.
//

#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>
#include <random>
#include <chrono>
#include "crow.h"

namespace fs = std::filesystem;

struct FileData {
    std::string filename;
    std::string filepath;
    size_t size;
    std::chrono::steady_clock::time_point expires_at;
};

std::map<std::string, FileData> file_storage;
const std::string TEMP_DIR = "/tmp/fastdrop/";

std::string gen_code() {
    std::vector<std::string> adjectives = {"purple", "fast", "blue", "happy", "lazy", "wild"};
    std::vector<std::string> nouns1 = {"disco", "ninja", "space", "cyber", "turbo", "mega"};
    std::vector<std::string> nouns2 = {"panda", "fox", "tiger", "eagle", "wolf", "shark"};

    std::random_device rd;
    std::mt19937 gen(rd());

    return adjectives[gen() % adjectives.size()] + "-" +
           nouns1[gen() % nouns1.size()] + "-" +
           nouns2[gen() % nouns2.size()];
}

int main() {
    crow::SimpleApp app;

    fs::create_directories(TEMP_DIR);

    CROW_ROUTE(app, "/upload").methods("POST"_method)
    ([](const crow::request& req) {
        crow::multipart::message msg(req);

        for (const auto& part : msg.part_map) {
            if (part.first == "file") {
                auto& file_part = part.second;
                std::string filename = file_part.get_header_object("Content-Disposition").params.at("filename");

                std::string code = gen_code();
                std::string filepath = TEMP_DIR + code + "_" + filename;

                std::ofstream ofs(filepath, std::ios::binary);
                ofs << file_part.body;
                ofs.close();

                file_storage[code] = {
                    filename,
                    filepath,
                    file_part.body.size(),
                    std::chrono::steady_clock::now() + std::chrono::minutes(10)
                };

                crow::json::wvalue response;
                response["code"] = code;
                response["expires_in"] = 600;
                response["qr_url"] = "https://api.qrserver.com/v1/create-qr-code/?data=" + code;
                return crow::response(200, response);
            }
        }
        return crow::response(400, "No file provided");
    });

    CROW_ROUTE(app, "/status/<string>")
    ([](const std::string& code) {
        crow::json::wvalue response;

        if (file_storage.find(code) != file_storage.end()) {
            auto& data = file_storage[code];
            response["found"] = true;
            response["filename"] = data.filename;
            response["size"] = std::to_string(data.size / 1024) + "KB";
            response["ai_summary"] = ""; // placeeholder
        } else {
            response["found"] = false;
        }
        return response;
    });

    CROW_ROUTE(app, "/retrieve/<string>")
    ([](const std::string& code) {
        if (file_storage.find(code) == file_storage.end()) {
            return crow::response(404, "File not found");
        }

        auto& data = file_storage[code];
        std::ifstream ifs(data.filepath, std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(ifs)),
                            std::istreambuf_iterator<char>());
        ifs.close();

        fs::remove(data.filepath);
        file_storage.erase(code);

        crow::response res(200);
        res.set_header("Content-Type", "application/octet-stream");
        res.set_header("Content-Disposition", "attachment; filename=\"" + data.filename + "\"");
        res.body = content;
        return res;
    });

    std::cout << "FastDrop server running on http://localhost:18080" << std::endl;
    app.port(18080).multithreaded().run();
    return 0;
}
