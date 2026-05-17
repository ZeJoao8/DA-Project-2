#include "BatchMode.h"
#include "../utils/DataLoader.h"
#include "../utils/GraphBuilder.h"
#include "../algorithms/GraphColoring.h"
#include "../algorithms/FreeAlgorithm.h"
#include "../utils/OutputWriter.h"
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
            
            // Run Register Allocation based on algorithm name
            ColoringResult result;
            if (registerConfig.algorithmName == "free") {
                std::cout << "Running Kempe Chain Register Allocation (free)...\n";
                result = FreeAlgorithm::run(interferenceGraph, webs, registerConfig.numRegisters);
            } else {
                // Default: basic greedy coloring
                std::cout << "Running Basic Register Allocation...\n";
                result = GraphColoring::run(interferenceGraph, webs, registerConfig.numRegisters, {});
            }

            // Write Results
            if (OutputWriter::writeResult(outputFile, webs, result, registerConfig.numRegisters)) {
                std::cout << "Results successfully written to " << outputFile << "\n";
            } else {
                std::cerr << "Error: Could not write results to " << outputFile << "\n";
            }

            if (!result.success) {
                std::cerr << "WARNING: Register allocation with " 
                          << registerConfig.numRegisters 
                          << " register(s) was not possible.\n";
            } else {
                std::cout << "Allocation successful using " << result.numColorsUsed << " register(s).\n";
            }

        } catch (const std::exception& e) {
            std::cerr << "Error: Failed during batch mode execution: " << e.what() << "\n";
        }
    }
}
