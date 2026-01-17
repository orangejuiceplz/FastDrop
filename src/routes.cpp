//
// Created by sakuya on 1/17/26.
//

#include "routes.hpp"
#include "file_storage.hpp"
#include "code_gen.hpp"
#include "config.hpp"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace fastdrop {

    void register_routes(crow::SimpleApp& app) {
        CROW_ROUTE(app, "/upload").methods("POST"_method)
        ([](const crow::request& req) {
            crow::multipart::message msg(req);

            for (const auto& part : msg.part_map) {
                if (part.first == "file") {
                    auto& file_part = part.second;
                    std::string filename = file_part.get_header_object("Content-Disposition").params.at("filename");

                    std::string code = genCode();
                    std::string filepath = TEMP_DIR + code + "_" + filename;

                    std::ofstream ofs(filepath, std::ios::binary);
                    ofs << file_part.body;
                    ofs.close();

                    FileData data{
                        filename,
                        filepath,
                        file_part.body.size(),
                        std::chrono::steady_clock::now() + std::chrono::seconds(DEFAULT_EXPIRY_SECONDS),
                        false,
                        ""
                    };
                    FileStorage::instance().store(code, data);

                    crow::json::wvalue response;
                    response["code"] = code;
                    response["expires_in"] = DEFAULT_EXPIRY_SECONDS;
                    response["qr_url"] = "https://api.qrserver.com/v1/create-qr-code/?data=" + code;
                    return crow::response(200, response);
                }
            }
            return crow::response(400, "No file provided");
        });

        CROW_ROUTE(app, "/status/<string>")
        ([](const std::string& code) {
            crow::json::wvalue response;

            auto data = FileStorage::instance().get(code);
            if (data.has_value()) {
                response["found"] = true;
                response["filename"] = data->filename;
                response["size"] = std::to_string(data->size / 1024) + "KB";
                response["ai_summary"] = ""; // placeholder
            } else {
                response["found"] = false;
            }
            return response;
        });

        CROW_ROUTE(app, "/retrieve/<string>")
        ([](const std::string& code) {
            auto data = FileStorage::instance().get(code);
            if (!data.has_value()) {
                return crow::response(404, "File not found");
            }

            std::ifstream ifs(data->filepath, std::ios::binary);
            std::string content((std::istreambuf_iterator<char>(ifs)),
                                std::istreambuf_iterator<char>());
            ifs.close();

            std::string filename = data->filename;
            FileStorage::instance().remove(code);

            crow::response res(200);
            res.set_header("Content-Type", "application/octet-stream");
            res.set_header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
            res.body = content;
            return res;
        });

        CROW_ROUTE(app, "/")
        ([]() {
            return "FastDrop is running!";
        });
        CROW_ROUTE(app, "/health")
        ([]() {
            return "OK";
        });
    }

}