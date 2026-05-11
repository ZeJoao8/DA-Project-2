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
    bool isDef; //se é a linha onde foi definido
    bool isEnd; //se é a linha onde morreu
};

struct VariableRanges {
    std::string varName;
    std::vector<std::vector<LiveRangePoint>> ranges;
};

class DataLoader {
public:
    /**
     * Resolves short filenames to full paths by checking common directories.
     * @param inputPath The filename or path.
     * @param defaultDir The folder inside basic/ where this file usually resides ("ranges" or "registers").
     * @return The resolved path.
     */
    static std::string resolvePath(const std::string& inputPath, const std::string& defaultDir);

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
