#ifndef DATALOADER_H
#define DATALOADER_H

#include <string>
#include <vector>
#include <unordered_map>

struct RegisterConfig {
    int numRegisters = 0;
    std::string algorithmName;
    int algorithmParam = 0;
};

struct LiveRangePoint {
    int line;
    bool isDef;
    bool isEnd;
};

struct VariableRanges {
    std::string varName;
    std::vector<std::vector<LiveRangePoint>> ranges;
};

class DataLoader {
public:
    /**
     * Parses the registers.txt file to extract register and algorithm configuration.
     * @param filename Path to the configuration file.
     * @return RegisterConfig struct containing the parsed data.
     * @throws std::runtime_error if the file cannot be opened or has a bad format.
     */
    static RegisterConfig loadRegisters(const std::string& filename);

    /**
     * Parses the ranges.txt file to extract live range points for variables.
     * @param filename Path to the ranges file.
     * @return An unordered_map mapping variable names to their parsed ranges.
     * @throws std::runtime_error if the file cannot be opened or has a bad format.
     */
    static std::unordered_map<std::string, VariableRanges> loadRanges(const std::string& filename);
};

#endif // DATALOADER_H
