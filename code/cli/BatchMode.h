/**
 * @file BatchMode.h
 * @brief Declares BatchMode::run() — the non-interactive allocation pipeline.
 *
 * Batch mode is triggered via the `-b` command-line flag and executes the
 * full allocation pipeline (load → build webs → build graph → allocate →
 * write output) in a single pass without user interaction.
 *
 */

#ifndef BATCHMODE_H
#define BATCHMODE_H

#include <string>

/**
 * @brief Non-interactive register allocation pipeline.
 */
namespace BatchMode {
    /**
     * @brief Execute the complete register allocation pipeline in batch mode.
     *
     * Loads input files, constructs webs and the interference graph, runs
     * the allocation algorithm specified in @p registersFile, and writes
     * the result to @p outputFile. Any exception during loading or
     * processing is caught and reported to stderr.
     *
     * @param rangesFile    Path to the ranges input file (e.g., `ranges.txt`).
     * @param registersFile Path to the registers/algorithm config file.
     * @param outputFile    Path where the allocation result will be written.
     *
     * @complexity Dominated by AllocationEngine::runAllocation():
     *   O(W^2 * L) for graph construction + O(V + E) to O(maxSplits * V^2)
     *   for allocation, depending on the chosen algorithm.
     */
    void run(const std::string& rangesFile,
             const std::string& registersFile,
             const std::string& outputFile);
}

#endif // BATCHMODE_H
