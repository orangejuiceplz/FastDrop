#include "session_manager.hpp"
#include "code_gen.hpp"
#include <algorithm>

namespace fastdrop {

    SessionManager& SessionManager::instance() {
        static SessionManager instance;
        return instance;
    }

    std::string SessionManager::create_session() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string session_id = genCode();
        session_files_[session_id] = {};
        return session_id;
    }

    bool SessionManager::session_exists(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        return session_files_.find(session_id) != session_files_.end();
    }

    void SessionManager::add_file_to_session(const std::string& session_id, const SharedFile& file) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (session_files_.find(session_id) != session_files_.end()) {
            session_files_[session_id].push_back(file);
        }
    }

    std::vector<SharedFile> SessionManager::get_session_files(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (session_files_.find(session_id) != session_files_.end()) {
            return session_files_[session_id];
        }
        return {};
    }

    void SessionManager::remove_file_from_session(const std::string& session_id, const std::string& code) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (session_files_.find(session_id) != session_files_.end()) {
            auto& files = session_files_[session_id];
            files.erase(
                std::remove_if(files.begin(), files.end(),
                    [&code](const SharedFile& f) { return f.code == code; }),
                files.end()
            );
        }
    }

    void SessionManager::add_connection(const std::string& session_id, crow::websocket::connection* conn) {
        std::lock_guard<std::mutex> lock(mutex_);
        session_connections_[session_id].insert(conn);
        if (session_files_.find(session_id) == session_files_.end()) {
            session_files_[session_id] = {};
        }
    }

    void SessionManager::remove_connection(const std::string& session_id, crow::websocket::connection* conn) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (session_connections_.find(session_id) != session_connections_.end()) {
            session_connections_[session_id].erase(conn);
        }
    }

    void SessionManager::broadcast_to_session(const std::string& session_id, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (session_connections_.find(session_id) != session_connections_.end()) {
            for (auto* conn : session_connections_[session_id]) {
                try {
                    conn->send_text(message);
                } catch (...) {}
            }
        }
    }

    void SessionManager::cleanup_empty_sessions() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = session_connections_.begin(); it != session_connections_.end(); ) {
            if (it->second.empty()) {
                session_files_.erase(it->first);
                it = session_connections_.erase(it);
            } else {
                ++it;
            }
        }
    }

}
