#ifndef GRAPHCOLORING_H
#define GRAPHCOLORING_H

#include "../data_structures/Graph.h"
#include "../models/Web.h"
#include <vector>
#include <unordered_map>

/**
 * @brief Result of a graph coloring attempt.
 */
struct ColoringResult {
    bool success;                               ///< True if coloring succeeded with no forced spills
    int numColorsUsed;                          ///< Number of distinct registers actually assigned
    std::unordered_map<int, int> webToRegister; ///< web.id -> register index (-1 = memory)
    std::vector<int> spilledWebs;              ///< IDs of webs that were spilled
    std::vector<Web> splitWebs;                ///< Updated webs after splitting (empty if no splitting)
};

/**
 * @brief Greedy graph coloring algorithm for register allocation.
 *
 * Phase 1 - Simplification: iteratively remove nodes with degree < N onto a stack.
 *   If all remaining nodes have degree >= N, spill the highest-degree node.
 * Phase 2 - Coloring: pop stack and assign the lowest available color not used by neighbors.
 *
 * Time Complexity: O(V^2) where V = number of webs.
 */
class GraphColoring {
public:
    /**
     * @brief Run the greedy coloring on the interference graph.
     *
     * @param graph          Interference graph with web IDs as vertex info.
     * @param webs           All webs (for metadata).
     * @param numRegisters   Number of available registers (N colors).
     * @param spilledWebIds  Web IDs pre-decided as spilled (excluded from coloring).
     * @return ColoringResult with success flag and web->register mapping.
     *
     * Time Complexity: O(V^2)
     */
    static ColoringResult run(
        const Graph<int>& graph,
        const std::vector<Web>& webs,
        int numRegisters,
        const std::vector<int>& spilledWebIds = {}
    );

    /**
     * @brief Select the best spill candidate: highest-degree node among active nodes.
     *
     * @param graph       Interference graph.
     * @param excludedIds Web IDs to skip (already spilled or on stack).
     * @param activeIds   Web IDs currently active in the working graph.
     * @return Web ID of best candidate, or -1 if none found.
     *
     * Time Complexity: O(V + E)
     */
    static int selectSpillCandidate(
        const Graph<int>& graph,
        const std::vector<int>& excludedIds,
        const std::vector<int>& activeIds
    );

private:
    /**
     * @brief Assign the lowest color not used by any neighbor.
     *
     * @param neighborColors Colors already used by neighbors.
     * @param numColors      Max colors available.
     * @return Lowest free color, or -1 if all taken.
     */
    static int assignColor(
        const std::vector<int>& neighborColors,
        int numColors
    );
};

#endif // GRAPHCOLORING_H