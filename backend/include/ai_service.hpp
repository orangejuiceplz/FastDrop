//
// Created by orangejuiceplz on 1/17/26.
//

#pragma once
#include <string>
#include <optional>

namespace fastdrop {

    class AIService {
    public:
        static std::optional<std::string> summarize(const std::string& content);
        static bool is_text_file(const std::string& filename);
        static void set_api_key(const std::string& key);
        
    private:
        static std::string api_key_;
    };

}
