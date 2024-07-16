#pragma once

#include <fstream>
#include <filesystem>
#include <vector>
#include <ctime>

#include "log.hpp"

constexpr unsigned long hash(const char* str) {
    unsigned long hash = 5381;
    int c = 0;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (file.is_open()) {
        file << content;
        file.close();
    } else {
        error("Unable to open file for writing: " + path);
        exit(EXIT_FAILURE);
    }
}

std::vector<std::string> getSubVector(const std::vector<std::string>& vec, std::size_t start, std::size_t end) {
    if (end > vec.size())
        end = vec.size() - 1;
    return std::vector<std::string>(vec.begin() + start, vec.begin() + end);
}

std::chrono::milliseconds::rep getElapsed(std::chrono::_V2::system_clock::time_point _start) {
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = end - _start;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsedSeconds).count();
}
