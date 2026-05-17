#include "AllocationEngine.h"
#include "../algorithms/GraphColoring.h"
#include "../algorithms/SpillingAlgorithm.h"
#include "../algorithms/Splitting.h"
#include "../algorithms/FreeAlgorithm.h"
#include "../io/OutputWriter.h"
#include <iostream>

namespace AllocationEngine {
    void runAllocation(
        const Graph<int>& interferenceGraph,
        const std::vector<Web>& webs,
        const RegisterConfig& registerConfig,
        const std::string& outputFile
    ) {
        std::cout << "\nAlgorithm: " << registerConfig.algorithmName
                  << " | Registers: " << registerConfig.numRegisters
                  << " | Param: " << registerConfig.algorithmParam << "\n";

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
    }
}
