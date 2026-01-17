#pragma once
#include <string>
#include <map>
#include <set>
#include <vector>
#include <mutex>
#include <functional>
#include "crow.h"

namespace fastdrop {

    struct SharedFile {
        std::string code;
        std::string filename;
        std::string size;
        bool locked;
    };

    class SessionManager {
    public:
        static SessionManager& instance();

        std::string create_session();
        bool session_exists(const std::string& session_id);
        void add_file_to_session(const std::string& session_id, const SharedFile& file);
        std::vector<SharedFile> get_session_files(const std::string& session_id);
        void remove_file_from_session(const std::string& session_id, const std::string& code);
        void add_connection(const std::string& session_id, crow::websocket::connection* conn);
        void remove_connection(const std::string& session_id, crow::websocket::connection* conn);
        void broadcast_to_session(const std::string& session_id, const std::string& message);
        void cleanup_empty_sessions();

    private:
        std::map<std::string, std::vector<SharedFile>> session_files_;
        std::map<std::string, std::set<crow::websocket::connection*>> session_connections_;
        std::mutex mutex_;
    };

}
