#include "BatchMode.h"
#include "../utils/DataLoader.h"
#include "../utils/GraphBuilder.h"
#include "../algorithms/GraphColoring.h"
#include "../algorithms/SpillingAlgorithm.h"
#include "../algorithms/Splitting.h"
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
            
            // Always run basic coloring first
            std::cout << "Running Basic Register Allocation...\n";
            ColoringResult result = GraphColoring::run(
                interferenceGraph, webs, registerConfig.numRegisters, {});

            if (result.success) {
                // Basic succeeded — print and write output
                std::cout << "Allocation successful using " << result.numColorsUsed << " register(s).\n";
                if (OutputWriter::writeResult(outputFile, webs, result, registerConfig.numRegisters)) {
                    std::cout << "Results successfully written to " << outputFile << "\n";
                } else {
                    std::cerr << "Error: Could not write results to " << outputFile << "\n";
                }
            } else {
                // Basic failed — dispatch based on algorithmName
                if (registerConfig.algorithmName == "basic") {
                    std::cerr << "WARNING: Register allocation not possible with "
                              << registerConfig.numRegisters << " register(s).\n";
                    if (OutputWriter::writeResult(outputFile, webs, result, registerConfig.numRegisters)) {
                        std::cout << "Results written to " << outputFile << "\n";
                    } else {
                        std::cerr << "Error: Could not write results to " << outputFile << "\n";
                    }
                } else if (registerConfig.algorithmName == "spilling") {
                    std::cout << "Basic allocation failed. Attempting spilling...\n";
                    ColoringResult spillResult = SpillingAlgorithm::run(
                        interferenceGraph, webs, registerConfig.numRegisters, registerConfig.algorithmParam);

                    if (spillResult.success) {
                        std::cout << "Spilling succeeded using " << spillResult.numColorsUsed << " register(s).\n";
                    } else {
                        std::cerr << "WARNING: Spilling failed. Some webs assigned to memory.\n";
                    }
                    if (OutputWriter::writeResult(outputFile, webs, spillResult, registerConfig.numRegisters)) {
                        std::cout << "Results written to " << outputFile << "\n";
                    } else {
                        std::cerr << "Error: Could not write results to " << outputFile << "\n";
                    }
                } else if (registerConfig.algorithmName == "splitting") {
                    std::cout << "Basic allocation failed. Attempting splitting...\n";
                    ColoringResult splitResult = Splitting::runWithSplitting(
                        interferenceGraph, webs, registerConfig.numRegisters, registerConfig.algorithmParam);

                    if (splitResult.success) {
                        std::cout << "Splitting succeeded using " << splitResult.numColorsUsed << " register(s).\n";
                    } else {
                        std::cerr << "WARNING: Splitting failed. Some webs assigned to memory.\n";
                    }
                    if (OutputWriter::writeResult(outputFile, splitResult.splitWebs, splitResult, registerConfig.numRegisters)) {
                        std::cout << "Results written to " << outputFile << "\n";
                    } else {
                        std::cerr << "Error: Could not write results to " << outputFile << "\n";
                    }
                } else if (registerConfig.algorithmName == "free") {
                    std::cout << "Basic allocation failed. Attempting Kempe Chain rescue (free)...\n";
                    ColoringResult freeResult = FreeAlgorithm::run(
                        interferenceGraph, webs, registerConfig.numRegisters);

                    if (freeResult.success) {
                        std::cout << "Free allocation succeeded using " << freeResult.numColorsUsed << " register(s).\n";
                    } else {
                        std::cerr << "WARNING: Free allocation could not avoid all spills.\n";
                    }
                    if (OutputWriter::writeResult(outputFile, webs, freeResult, registerConfig.numRegisters)) {
                        std::cout << "Results written to " << outputFile << "\n";
                    } else {
                        std::cerr << "Error: Could not write results to " << outputFile << "\n";
                    }
                } else {
                    std::cerr << "ERROR: Unknown algorithm '" << registerConfig.algorithmName
                              << "'. Supported: basic, spilling, splitting, free.\n";
                }
            }

        } catch (const std::exception& e) {
            std::cerr << "Error: Failed during batch mode execution: " << e.what() << "\n";
        }
    }
}
