#include "BatchMode.h"
#include <iostream>

namespace BatchMode {
    void run(const std::string& rangesFile, const std::string& registersFile, const std::string& outputFile) {
        std::cout << "--- Batch Mode Activated ---\n";
        std::cout << "Ranges File: " << rangesFile << "\n";
        std::cout << "Registers File: " << registersFile << "\n";
        std::cout << "Output File: " << outputFile << "\n";
        std::cout << "----------------------------\n";
    }
}
