#include "DataLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace {
    // Helper function to trim whitespace from both ends of a string
    std::string trim(const std::string& str) {
        auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char c) { return std::isspace(c); });
        auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) { return std::isspace(c); }).base();
        return (start < end) ? std::string(start, end) : std::string();
    }
}

std::string DataLoader::resolvePath(const std::string& inputPath, const std::string& defaultDir) {
    if (inputPath.find('/') != std::string::npos || inputPath.find('\\') != std::string::npos) {
        return inputPath;
    }

    std::ifstream currentDir(inputPath);
    if (currentDir.good()) return inputPath;

    std::string path1 = "../basic/" + defaultDir + "/" + inputPath;
    std::ifstream f1(path1);
    if (f1.good()) return path1;

    std::string path2 = "../../basic/" + defaultDir + "/" + inputPath;
    std::ifstream f2(path2);
    if (f2.good()) return path2;

    return path1; // fallback
}

RegisterConfig DataLoader::loadRegisters(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        throw std::runtime_error("Could not open file " + filename);
    }

    RegisterConfig config;
    std::string line;
    int linesParsed = 0;

    while (std::getline(file, line)) {
        line = trim(line);
        // Ignore empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, ':')) {
            key = trim(key);
            std::string valueStr;
            if (std::getline(iss, valueStr)) {
                valueStr = trim(valueStr);
                
                if (key == "registers") {
                    try {
                        config.numRegisters = std::stoi(valueStr);
                        linesParsed++;
                    } catch (const std::exception& e) {
                        std::cerr << "Error: Bad format for registers in " << filename << std::endl;
                        throw std::runtime_error("Bad format in " + filename);
                    }
                } else if (key == "algorithm") {
                    size_t commaPos = valueStr.find(',');
                    if (commaPos != std::string::npos) {
                        config.algorithmName = trim(valueStr.substr(0, commaPos));
                        try {
                            config.algorithmParam = std::stoi(trim(valueStr.substr(commaPos + 1)));
                        } catch (const std::exception& e) {
                            std::cerr << "Error: Bad format for algorithm parameter in " << filename << std::endl;
                            throw std::runtime_error("Bad format in " + filename);
                        }
                    } else {
                        config.algorithmName = valueStr;
                    }
                    linesParsed++;
                }
            }
        } else {
            std::cerr << "Error: Malformed line in " << filename << ": " << line << std::endl;
            throw std::runtime_error("Bad format in " + filename);
        }
    }

    if (linesParsed == 0) {
        std::cerr << "Error: File " << filename << " appears to be empty or malformed." << std::endl;
        throw std::runtime_error("Bad format in " + filename);
    }

    return config;
}

std::unordered_map<std::string, VariableRanges> DataLoader::loadRanges(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        throw std::runtime_error("Could not open file " + filename);
    }

    std::unordered_map<std::string, VariableRanges> rangesMap;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        // Ignore empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string varName;
        if (std::getline(iss, varName, ':')) {
            varName = trim(varName);
            if (varName.empty()) {
                std::cerr << "Error: Missing variable name in " << filename << std::endl;
                throw std::runtime_error("Malformed ranges file format");
            }

            std::string rangesStr;
            if (!std::getline(iss, rangesStr)) {
                std::cerr << "Error: Missing ranges for variable '" << varName << "' in " << filename << std::endl;
                throw std::runtime_error("Malformed ranges file format");
            }
            
            std::vector<LiveRangePoint> currentRanges;
            std::istringstream rss(rangesStr);
            std::string token;
            
            while (std::getline(rss, token, ',')) {
                token = trim(token);
                if (token.empty()) continue;

                LiveRangePoint point = {0, false, false};
                
                if (token.back() == '+') {
                    point.isDef = true;
                    token.pop_back();
                } else if (token.back() == '-') {
                    point.isEnd = true;
                    token.pop_back();
                }
                
                try {
                    point.line = std::stoi(token);
                    currentRanges.push_back(point);
                } catch (const std::exception& e) {
                    std::cerr << "Error: Malformed line number '" << token << "' for variable '" << varName << "' in " << filename << std::endl;
                    throw std::runtime_error("Malformed ranges file format");
                }
            }
            
            if (!currentRanges.empty()) {
                auto& varRanges = rangesMap[varName];
                if (varRanges.varName.empty()) {
                    varRanges.varName = varName;
                }
                varRanges.ranges.push_back(currentRanges);
            }
        } else {
            std::cerr << "Error: Malformed line in " << filename << ": " << line << std::endl;
            throw std::runtime_error("Malformed ranges file format");
        }
    }

    return rangesMap;
}
