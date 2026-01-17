//
// Created by sakuya on 1/17/26.
//

#include "ai_service.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iostream>

namespace fastdrop {

    std::string AIService::api_key_;

    void AIService::set_api_key(const std::string& key) {
        api_key_ = key;
    }

    bool AIService::is_text_file(const std::string& filename) {
        const std::vector<std::string> text_extensions = {
            ".txt", ".md", ".json", ".xml", ".csv", 
            ".html", ".css", ".js", ".cpp", ".hpp", 
            ".c", ".h", ".py", ".java", ".log",
            ".yaml", ".yml", ".ini", ".cfg",
            ".pdf"
        };
        
        std::string lower_filename = filename;
        std::transform(lower_filename.begin(), lower_filename.end(), 
                       lower_filename.begin(), ::tolower);
        
        for (const auto& ext : text_extensions) {
            if (lower_filename.size() >= ext.size() &&
                lower_filename.substr(lower_filename.size() - ext.size()) == ext) {
                return true;
            }
        }
        return false;
    }

    std::optional<std::string> AIService::summarize(const std::string& content) {
        if (api_key_.empty()) {
            return std::nullopt;
        }

        if (content.empty() || content.size() > 10000) {
            return std::nullopt;
        }

        try {
            nlohmann::json request_body = {
                {"contents", {
                    {{"parts", {
                        {{"text", "Summarize the following text in 1-2 sentences. If the text contains sensitive information, just say sensitive content is detected and a summary cannot be produced.\n\n" + content}}
                    }}}
                }}
            };

            std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + api_key_;

            cpr::Response response = cpr::Post(
                cpr::Url{url},
                cpr::Header{
                    {"Content-Type", "application/json"}
                },
                cpr::Body{request_body.dump()},
                cpr::Timeout{10000}
            );

            if (response.status_code != 200) {
                std::cout << "Gemini API error: " << response.status_code << " - " << response.text << std::endl;
                return std::nullopt;
            }

            nlohmann::json response_json = nlohmann::json::parse(response.text);
            std::string summary = response_json["candidates"][0]["content"]["parts"][0]["text"];
            return summary;

        } catch (const std::exception& error) {
            std::cout << "We got an error while summarizing: " << error.what() << std::endl;
            return std::nullopt;
        }
    }

}
