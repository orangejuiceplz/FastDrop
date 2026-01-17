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

    void register_routes(crow::App<crow::CORSHandler>& app) {
        CROW_ROUTE(app, "/upload").methods("POST"_method)
        ([](const crow::request& request) {
            crow::multipart::message multipart(request);

            for (const auto& part : multipart.part_map) {
                if (part.first == "file") {
                    auto& uploaded_file = part.second;
                    std::string filename = uploaded_file.get_header_object("Content-Disposition").params.at("filename");

                    std::string code = genCode();
                    std::string filepath = TEMP_DIR + code + "_" + filename;

                    std::ofstream output_file(filepath, std::ios::binary);
                    output_file << uploaded_file.body;
                    output_file.close();

                    FileData file_data{
                        filename,
                        filepath,
                        uploaded_file.body.size(),
                        std::chrono::steady_clock::now() + std::chrono::seconds(DEFAULT_EXPIRY_SECONDS),
                        false,
                        ""
                    };
                    FileStorage::instance().store(code, file_data);

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

            auto file_info = FileStorage::instance().get(code);
            if (file_info.has_value()) {
                response["found"] = true;
                response["filename"] = file_info->filename;
                response["size"] = std::to_string(file_info->size / 1024) + "KB";
                response["ai_summary"] = "";
            } else {
                response["found"] = false;
            }
            return response;
        });

        CROW_ROUTE(app, "/retrieve/<string>")
        ([](const std::string& code) {
            auto file_info = FileStorage::instance().get(code);
            if (!file_info.has_value()) {
                return crow::response(404, "File not found");
            }

            std::ifstream input_file(file_info->filepath, std::ios::binary);
            std::string file_content((std::istreambuf_iterator<char>(input_file)),
                                     std::istreambuf_iterator<char>());
            input_file.close();

            std::string filename = file_info->filename;
            FileStorage::instance().remove(code);

            crow::response download_response(200);
            download_response.set_header("Content-Type", "application/octet-stream");
            download_response.set_header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
            download_response.body = file_content;
            return download_response;
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