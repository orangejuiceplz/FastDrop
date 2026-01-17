//
// Created by sakuya on 1/17/26.
//

#include "code_gen.hpp"
#include <vector>
#include <random>

namespace fastdrop {

    std::string genCode() {
        std::vector<std::string> adjectives = {"cool", "lazy", "git", "charming", "sad", "happy"};
        std::vector<std::string> nouns = {"octopus", "snake", "cat", "hippo", "goat", "sheep"};
        std::vector<std::string> stringNum = {"67", "29", "19", "94", "10", "39"};

        std::random_device rd;
        std::mt19937 gen(rd());

        return adjectives[gen() % adjectives.size()] + "-" +
               nouns[gen() % nouns.size()] + "-" +
               stringNum[gen() % stringNum.size()];
    }

}