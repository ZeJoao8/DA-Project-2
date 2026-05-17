/**
 * @file AllocationEngine.h
 * @brief Declares AllocationEngine — the central dispatcher for all
 *        register allocation algorithms.
 *
 * AllocationEngine::runAllocation() is the single interface used by both
 * Menu (interactive) and BatchMode to run the correct allocation strategy
 * based on the parsed RegisterConfig.
 *
 * Supported algorithms:
 *   - `"basic"`    — Greedy graph coloring (GraphColoring).
 *   - `"spilling"` — Chaitin-style spilling (SpillingAlgorithm).
 *   - `"splitting"` — Iterative web splitting (Splitting).
 *   - `"free"`     — Kempe Chain rescue (FreeAlgorithm).
 *
 */

#ifndef ALLOCATIONENGINE_H
#define ALLOCATIONENGINE_H

#include "../models/RegisterConfig.h"
#include "../models/Web.h"
#include "../data_structures/Graph.h"
#include <string>
#include <vector>

namespace AllocationEngine {
    /**
     * @brief Centralized algorithm dispatching, success checking, and output writing.
     *
     * Always attempts basic greedy coloring first. If basic coloring succeeds,
     * the result is written and the function returns. On failure, the algorithm
     * specified by `registerConfig.algorithmName` is invoked as a fallback.
     *
     * Supported fallback algorithms and their trigger condition:
     *   - `"basic"`    : Reports failure; writes result with all webs in memory.
     *   - `"spilling"` : Runs Chaitin-style spilling (SpillingAlgorithm) with
     *                    `registerConfig.algorithmParam` as the spill limit.
     *   - `"splitting"`: Runs iterative web splitting (Splitting) with
     *                    `registerConfig.algorithmParam` as the split limit.
     *   - `"free"`     : Runs Kempe Chain rescue (FreeAlgorithm).
     *
     * @param interferenceGraph  The prebuilt interference graph (vertices = web IDs).
     * @param webs               List of all webs in the current allocation problem.
     * @param registerConfig     Configuration: number of registers, algorithm, and parameter.
     * @param outputFile         Destination path for the allocation result file.
     *
     * @pre @p interferenceGraph must be correctly built from @p webs, meaning each vertex
     *      info corresponds exactly to the `id` of a Web in @p webs.
     *
     * @complexity Dominated by the selected algorithm:
     *   - basic     : O(V + E)
     *   - spilling  : O(V + E)
     *   - splitting : O(maxSplits * (V^2 + E))
     *   - free      : O(V^2 + V * N^2 * (V + E))
     */
    void runAllocation(
        const Graph<int>& interferenceGraph,
        const std::vector<Web>& webs,
        const RegisterConfig& registerConfig,
        const std::string& outputFile
    );
}

#endif // ALLOCATIONENGINE_H
