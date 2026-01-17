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
        auto entry = storage_.find(code);
        if (entry != storage_.end()) {
            return entry->second;
        }
        return std::nullopt;
    }

    bool FileStorage::remove(const std::string& code) {
        auto entry = storage_.find(code);
        if (entry != storage_.end()) {
            fs::remove(entry->second.filepath);
            storage_.erase(entry);
            return true;
        }
        return false;
    }

    void FileStorage::cleanup_expired() {
        auto now = std::chrono::steady_clock::now();
        for (auto entry = storage_.begin(); entry != storage_.end(); ) {
            bool is_expired = entry->second.expires_at < now;
            bool is_temporary = !entry->second.persistent;
            
            if (is_temporary && is_expired) {
                fs::remove(entry->second.filepath);
                entry = storage_.erase(entry);
            } else {
                ++entry;
            }
        }
    }

}