//
// Created by orangejuiceplz on 1/17/26.
//

#pragma once
#include <string>
#include <map>
#include <optional>
#include <chrono>

namespace fastdrop {

    struct FileData {
        std::string filename;
        std::string filepath;
        size_t size;
        std::chrono::steady_clock::time_point expires_at;
        bool persistent = false;
        std::string key_hash; // this is for the file locking
    };

    class FileStorage {
    public:
        static FileStorage& instance();

        void store(const std::string& code, FileData data);
        std::optional<FileData> get(const std::string& code);
        bool remove(const std::string& code);
        void cleanup_expired();

    private:
        std::map<std::string, FileData> storage_;
    };

}

