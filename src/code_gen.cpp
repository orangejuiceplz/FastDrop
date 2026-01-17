//
// Created by sakuya on 1/17/26.
//

#include "code_gen.hpp"
#include <vector>
#include <random>

namespace fastdrop {

    std::string genCode() {
        std::vector<std::string> adjectives = {"purple", "fast", "blue", "happy", "lazy", "wild"};
        std::vector<std::string> nouns1 = {"disco", "ninja", "space", "cyber", "turbo", "mega"};
        std::vector<std::string> nouns2 = {"panda", "fox", "tiger", "eagle", "wolf", "shark"};

        std::random_device rd;
        std::mt19937 gen(rd());

        return adjectives[gen() % adjectives.size()] + "-" +
               nouns1[gen() % nouns1.size()] + "-" +
               nouns2[gen() % nouns2.size()];
    }

}