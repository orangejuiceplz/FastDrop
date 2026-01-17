//
// Created by orangejuiceplz on 1/17/26.
//

#pragma once
#include <string>
#include <map>
#include <optional>
#include <chrono>
#include <functional>

namespace fastdrop {

    inline std::string hash_password(const std::string& password) {
        std::hash<std::string> hasher;
        return std::to_string(hasher(password));
    }

    struct FileData {
        std::string filename;
        std::string filepath;
        size_t size;
        std::chrono::steady_clock::time_point expires_at;
        bool persistent = false;
        std::string key_hash;
        bool is_locked() const { return !key_hash.empty(); }
    };

    class FileStorage {
    public:
        static FileStorage& instance();

        void store(const std::string& code, FileData data);
        std::optional<FileData> get(const std::string& code);
        bool remove(const std::string& code);
        void cleanup_expired();
        bool verify_key(const std::string& code, const std::string& password);

    private:
        std::map<std::string, FileData> storage_;
    };

}

