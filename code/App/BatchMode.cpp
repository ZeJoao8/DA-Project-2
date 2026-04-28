#include "BatchMode.h"
#include "../utils/DataLoader.h"
#include <iostream>

namespace BatchMode {
    void run(const std::string& rangesFile, const std::string& registersFile, const std::string& outputFile) {
        std::cout << "--- Batch Mode Activated ---\n";
        
        std::string rPath = DataLoader::resolvePath(rangesFile, "ranges");
        std::string regPath = DataLoader::resolvePath(registersFile, "registers");

        std::cout << "Ranges File: " << rPath << "\n";
        std::cout << "Registers File: " << regPath << "\n";
        std::cout << "Output File: " << outputFile << "\n";
        std::cout << "----------------------------\n";

        try {
            std::cout << "Loading data...\n";
            auto rangesMap = DataLoader::loadRanges(rPath);
            auto registerConfig = DataLoader::loadRegisters(regPath);
            std::cout << "Successfully loaded " << rangesMap.size() << " variables and " 
                      << registerConfig.numRegisters << " registers.\n";
            
            // Algorithms and output writing will run here later...

        } catch (const std::exception& e) {
            std::cerr << "Error: Failed to load data in batch mode.\n";
        }
    }
}
