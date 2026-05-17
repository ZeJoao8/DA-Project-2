/**
 * @file BatchMode.cpp
 * @brief Implements the non-interactive batch allocation pipeline.
 *
 * Drives the full pipeline — load → buildWebs → buildInterferenceGraph →
 * runAllocation — without any user prompts. Exceptions are caught and
 * reported to stderr so the process exits cleanly.
 *
 */

#include "BatchMode.h"
#include "../io/DataLoader.h"
#include "../core/GraphBuilder.h"
#include "../core/AllocationEngine.h"
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
            
            // Build Webs
            std::vector<Web> webs = GraphBuilder::buildWebs(rangesMap);
            std::cout << "Generated " << webs.size() << " webs.\n";

            // Build Interference Graph
            Graph<int> interferenceGraph = GraphBuilder::buildInterferenceGraph(webs);
            
            AllocationEngine::runAllocation(interferenceGraph, webs, registerConfig, outputFile);

        } catch (const std::exception& e) {
            std::cerr << "Error: Failed during batch mode execution: " << e.what() << "\n";
        }
    }
}
