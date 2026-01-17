//
// Created by sakuya on 1/17/26.
//

#include "file_storage.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace fastdrop {

    FileStorage& FileStorage::instance() {
        static FileStorage instance;
        return instance;
    }

    void FileStorage::store(const std::string& code, FileData data) {
        storage_[code] = std::move(data);
    }

    std::optional<FileData> FileStorage::get(const std::string& code) {
        auto it = storage_.find(code);
        if (it != storage_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool FileStorage::remove(const std::string& code) {
        auto it = storage_.find(code);
        if (it != storage_.end()) {
            fs::remove(it->second.filepath);
            storage_.erase(it);
            return true;
        }
        return false;
    }

    void FileStorage::cleanup_expired() {
        auto now = std::chrono::steady_clock::now();
        for (auto it = storage_.begin(); it != storage_.end(); ) {
            if (!it->second.persistent && it->second.expires_at < now) {
                fs::remove(it->second.filepath);
                it = storage_.erase(it);
            } else {
                ++it;
            }
        }
    }

}