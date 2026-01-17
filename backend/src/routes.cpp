//
// Created by sakuya on 1/17/26.
//

#include "routes.hpp"
#include "file_storage.hpp"
#include "code_gen.hpp"
#include "config.hpp"
#include "ai_service.hpp"
#include "qr_service.hpp"
#include "session_manager.hpp"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace fastdrop {

    crow::json::wvalue make_error(const std::string& message) {
        crow::json::wvalue error;
        error["error"] = message;
        return error;
    }

    void register_routes(crow::App<crow::CORSHandler>& app) {
        CROW_ROUTE(app, "/upload").methods("POST"_method)
        ([](const crow::request& request) {
            try {
                crow::multipart::message multipart(request);

                std::string password = "";
                for (const auto& part : multipart.part_map) {
                    if (part.first == "password") {
                        password = part.second.body;
                    }
                }

                for (const auto& part : multipart.part_map) {
                    if (part.first == "file") {
                        auto& uploaded_file = part.second;
                        
                        auto content_disposition = uploaded_file.get_header_object("Content-Disposition");
                        if (content_disposition.params.find("filename") == content_disposition.params.end()) {
                            return crow::response(400, make_error("Missing filename in upload"));
                        }
                        
                        std::string filename = content_disposition.params.at("filename");
                        if (filename.empty()) {
                            return crow::response(400, make_error("Filename cannot be empty"));
                        }

                        if (uploaded_file.body.empty()) {
                            return crow::response(400, make_error("File content is empty"));
                        }

                        constexpr size_t MAX_FILE_SIZE = 50 * 1024 * 1024; // 50MB
                        if (uploaded_file.body.size() > MAX_FILE_SIZE) {
                            return crow::response(413, make_error("File too large (max 50MB)"));
                        }

                        std::string code = genCode();
                        std::string filepath = TEMP_DIR + code + "_" + filename;

                        std::ofstream output_file(filepath, std::ios::binary);
                        if (!output_file.is_open()) {
                            return crow::response(500, make_error("Failed to save file"));
                        }
                        output_file << uploaded_file.body;
                        output_file.close();

                        if (output_file.fail()) {
                            fs::remove(filepath);
                            return crow::response(500, make_error("Failed to write file"));
                        }

                        std::string key_hash = password.empty() ? "" : hash_password(password);

                        FileData file_data{
                            filename,
                            filepath,
                            uploaded_file.body.size(),
                            std::chrono::steady_clock::now() + std::chrono::seconds(DEFAULT_EXPIRY_SECONDS),
                            false,
                            key_hash
                        };
                        FileStorage::instance().store(code, file_data);

                        crow::json::wvalue response;
                        response["code"] = code;
                        response["expires_in"] = DEFAULT_EXPIRY_SECONDS;
                        response["locked"] = !password.empty();
                        response["qr_url"] = "/qr/" + code;
                        return crow::response(200, response);
                    }
                }
                return crow::response(400, make_error("No file provided"));
            } catch (const std::exception& error) {
                return crow::response(500, make_error("Upload failed: " + std::string(error.what())));
            }
        });

        CROW_ROUTE(app, "/status/<string>")
        ([](const std::string& code) {
            crow::json::wvalue response;

            auto file_info = FileStorage::instance().get(code);
            if (file_info.has_value()) {
                response["found"] = true;
                response["filename"] = file_info->filename;
                response["size"] = std::to_string(file_info->size / 1024) + "KB";
                response["locked"] = file_info->is_locked();
                response["ai_summary"] = "";
            } else {
                response["found"] = false;
            }
            return response;
        });

        CROW_ROUTE(app, "/summarize/<string>")
        ([](const std::string& code) {
            crow::json::wvalue response;

            auto file_info = FileStorage::instance().get(code);
            if (!file_info.has_value()) {
                response["error"] = "File not found";
                return crow::response(404, response);
            }

            if (!AIService::is_text_file(file_info->filename)) {
                response["error"] = "AI summary only available for text files";
                return crow::response(400, response);
            }

            std::ifstream input_file(file_info->filepath);
            if (!input_file.is_open()) {
                response["error"] = "Failed to read file";
                return crow::response(500, response);
            }

            std::string file_content((std::istreambuf_iterator<char>(input_file)),
                                     std::istreambuf_iterator<char>());
            input_file.close();

            auto summary = AIService::summarize(file_content);
            if (summary.has_value()) {
                response["summary"] = summary.value();
            } else {
                response["summary"] = "Summary unavailable";
            }

            return crow::response(200, response);
        });

        CROW_ROUTE(app, "/retrieve/<string>").methods("POST"_method, "GET"_method)
        ([](const crow::request& request, const std::string& code) {
            auto file_info = FileStorage::instance().get(code);
            if (!file_info.has_value()) {
                return crow::response(404, "File not found");
            }

            if (file_info->is_locked()) {
                std::string password = "";
                
                if (request.method == crow::HTTPMethod::POST) {
                    try {
                        auto body = crow::json::load(request.body);
                        if (body && body.has("password")) {
                            password = body["password"].s();
                        }
                    } catch (...) {}
                }

                if (!FileStorage::instance().verify_key(code, password)) {
                    crow::json::wvalue error;
                    error["error"] = "Invalid password";
                    error["locked"] = true;
                    return crow::response(401, error);
                }
            }

            std::ifstream input_file(file_info->filepath, std::ios::binary);
            if (!input_file.is_open()) {
                return crow::response(500, "Failed to read file");
            }

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

        CROW_ROUTE(app, "/qr/<string>")
        ([](const std::string& code) {
            std::string svg = generate_qr_svg(code);
            
            crow::response qr_response(200);
            qr_response.set_header("Content-Type", "image/svg+xml");
            qr_response.body = svg;
            return qr_response;
        });

        CROW_ROUTE(app, "/session/create").methods("POST"_method)
        ([]() {
            std::string session_id = SessionManager::instance().create_session();
            crow::json::wvalue response;
            response["session_id"] = session_id;
            return crow::response(200, response);
        });

        CROW_ROUTE(app, "/session/<string>/files")
        ([](const std::string& session_id) {
            if (!SessionManager::instance().session_exists(session_id)) {
                crow::json::wvalue error;
                error["error"] = "Session not found";
                return crow::response(404, error);
            }

            auto files = SessionManager::instance().get_session_files(session_id);
            crow::json::wvalue response;
            std::vector<crow::json::wvalue> file_list;
            for (const auto& file : files) {
                crow::json::wvalue f;
                f["code"] = file.code;
                f["filename"] = file.filename;
                f["size"] = file.size;
                f["locked"] = file.locked;
                file_list.push_back(std::move(f));
            }
            response["files"] = std::move(file_list);
            return crow::response(200, response);
        });

        CROW_ROUTE(app, "/session/<string>/upload").methods("POST"_method)
        ([](const crow::request& request, const std::string& session_id) {
            if (!SessionManager::instance().session_exists(session_id)) {
                return crow::response(404, make_error("Session not found"));
            }

            try {
                crow::multipart::message multipart(request);

                std::string password = "";
                for (const auto& part : multipart.part_map) {
                    if (part.first == "password") {
                        password = part.second.body;
                    }
                }

                for (const auto& part : multipart.part_map) {
                    if (part.first == "file") {
                        auto& uploaded_file = part.second;
                        
                        auto content_disposition = uploaded_file.get_header_object("Content-Disposition");
                        if (content_disposition.params.find("filename") == content_disposition.params.end()) {
                            return crow::response(400, make_error("Missing filename in upload"));
                        }
                        
                        std::string filename = content_disposition.params.at("filename");
                        if (filename.empty()) {
                            return crow::response(400, make_error("Filename cannot be empty"));
                        }

                        if (uploaded_file.body.empty()) {
                            return crow::response(400, make_error("File content is empty"));
                        }

                        constexpr size_t MAX_FILE_SIZE = 50 * 1024 * 1024;
                        if (uploaded_file.body.size() > MAX_FILE_SIZE) {
                            return crow::response(413, make_error("File too large (max 50MB)"));
                        }

                        std::string code = genCode();
                        std::string filepath = TEMP_DIR + code + "_" + filename;

                        std::ofstream output_file(filepath, std::ios::binary);
                        if (!output_file.is_open()) {
                            return crow::response(500, make_error("Failed to save file"));
                        }
                        output_file << uploaded_file.body;
                        output_file.close();

                        std::string key_hash = password.empty() ? "" : hash_password(password);
                        std::string size_str = std::to_string(uploaded_file.body.size() / 1024) + "KB";

                        FileData file_data{
                            filename,
                            filepath,
                            uploaded_file.body.size(),
                            std::chrono::steady_clock::now() + std::chrono::seconds(DEFAULT_EXPIRY_SECONDS),
                            false,
                            key_hash
                        };
                        FileStorage::instance().store(code, file_data);

                        SharedFile shared{code, filename, size_str, !password.empty()};
                        SessionManager::instance().add_file_to_session(session_id, shared);

                        crow::json::wvalue broadcast_msg;
                        broadcast_msg["type"] = "file_added";
                        broadcast_msg["code"] = code;
                        broadcast_msg["filename"] = filename;
                        broadcast_msg["size"] = size_str;
                        broadcast_msg["locked"] = !password.empty();
                        SessionManager::instance().broadcast_to_session(session_id, broadcast_msg.dump());

                        crow::json::wvalue response;
                        response["code"] = code;
                        response["filename"] = filename;
                        response["size"] = size_str;
                        response["locked"] = !password.empty();
                        return crow::response(200, response);
                    }
                }
                return crow::response(400, make_error("No file provided"));
            } catch (const std::exception& error) {
                return crow::response(500, make_error("Upload failed: " + std::string(error.what())));
            }
        });

        CROW_ROUTE(app, "/ws/<string>")
            .websocket(&app)
            .onopen([](crow::websocket::connection& conn) {
                std::string url = conn.get_remote_ip();
            })
            .onclose([](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
                std::string path = std::string(conn.get_remote_ip());
            })
            .onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
                try {
                    auto json = crow::json::load(data);
                    if (json && json.has("action")) {
                        std::string action = json["action"].s();
                        if (action == "join" && json.has("session_id")) {
                            std::string session_id = json["session_id"].s();
                            SessionManager::instance().add_connection(session_id, &conn);
                            
                            auto files = SessionManager::instance().get_session_files(session_id);
                            crow::json::wvalue response;
                            response["type"] = "session_joined";
                            response["session_id"] = session_id;
                            std::vector<crow::json::wvalue> file_list;
                            for (const auto& file : files) {
                                crow::json::wvalue f;
                                f["code"] = file.code;
                                f["filename"] = file.filename;
                                f["size"] = file.size;
                                f["locked"] = file.locked;
                                file_list.push_back(std::move(f));
                            }
                            response["files"] = std::move(file_list);
                            conn.send_text(response.dump());
                        } else if (action == "leave" && json.has("session_id")) {
                            std::string session_id = json["session_id"].s();
                            SessionManager::instance().remove_connection(session_id, &conn);
                        }
                    }
                } catch (...) {}
            });
    }

}