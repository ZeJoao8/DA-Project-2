/**
 * @file FreeAlgorithm.h
 * @brief Declares FreeAlgorithm — a register allocator that uses Kempe Chain
 *        swaps as a last-resort rescue before falling back to spilling.
 *
 * Algorithm overview:
 *   - **Phase 1 (Simplification)**: same as GraphColoring — remove trivially
 *     colorable nodes; push non-trivial nodes optimistically onto the stack.
 *   - **Phase 2 (Coloring with rescue)**: pop and assign greedily. When a node
 *     is stuck (all K registers taken by neighbors), attempt a Kempe Chain
 *     swap to free a color. If rescue fails, the node is spilled.
 *
 * A *Kempe Chain* for colors (c1, c2) starting at node A is the maximal
 * subgraph reachable from A using only c1- and c2-colored nodes. Swapping
 * c1 &hArr; c2 within the chain is a safe recoloring that does not violate
 * the graph-coloring invariant, and frees color c1 for the stuck node.
 *
 */

#ifndef FREEALGORITHM_H
#define FREEALGORITHM_H

#include "../data_structures/Graph.h"
#include "../models/Web.h"
#include "GraphColoring.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * @brief Register allocation using Kempe Chain swaps as a last-resort rescue
 *        to avoid spilling.
 *
 * Runs the standard greedy graph coloring (Phase 1 simplification + Phase 2
 * coloring). When Phase 2 gets stuck (all N registers taken by neighbors of
 * a node), it attempts a Kempe Chain rescue before falling back to spilling.
 *
 * ### Kempe Chain Rescue
 * Given stuck node X with neighbors using every available color:
 *   1. Pick a target color `c` (used by neighbor A) and a partner color `c2`.
 *   2. Build the (c–c2) Kempe chain starting at A: the maximal connected
 *      subgraph containing only c- and c2-colored nodes that includes A.
 *   3. Validate: none of X's other `c2`-colored neighbors may be in the chain
 *      (swapping the chain would otherwise still leave X with a `c` neighbor).
 *   4. If valid, swap all colors in the chain (c ↔ c2). Color `c` is now
 *      free for X.
 */
class FreeAlgorithm {
public:
    /**
     * @brief Run register allocation with Kempe Chain rescue.
     *
     * Phase 1: same simplification as GraphColoring::run().
     * Phase 2: coloring with Kempe Chain rescue on failure before spilling.
     *
     * @param graph          Interference graph with web IDs as vertex info.
     * @param webs           All webs (for metadata).
     * @param numRegisters   Number of available registers (N colors).
     * @return ColoringResult with success flag and web->register mapping.
     *         result.success is true only if no node was ultimately spilled.
     *
     * @complexity O(V × N² × (V + E))
     */
    static ColoringResult run(
        const Graph<int>& graph,
        const std::vector<Web>& webs,
        int numRegisters
    );

private:
    /**
     * @brief Build the (c1–c2) Kempe chain starting at startId.
     *
     * BFS/DFS through the current colorAssignment, visiting only nodes colored
     * c1 or c2 that are reachable from startId via the interference graph.
     *
     * @param graph            Interference graph.
     * @param colorAssignment  Current partial color assignment (id -> color).
     * @param startId          Node ID to start the chain from.
     * @param c1               First color in the chain.
     * @param c2               Second color in the chain.
     * @return Set of node IDs in the Kempe chain.
     *
     * @complexity O(V + E)
     */
    static std::unordered_set<int> buildKempeChain(
        const Graph<int>& graph,
        const std::unordered_map<int, int>& colorAssignment,
        int startId,
        int c1,
        int c2
    );

    /**
     * @brief Attempt a Kempe Chain rescue for stuck node nodeId.
     *
     * Iterates over all pairs of colors (c, c2) used by neighbors of nodeId.
     * For each neighbor A with color c, builds the (c–c2) chain from A and
     * validates that no other c2-neighbor of nodeId is inside the chain.
     * On success, swaps the chain in-place and returns the freed color c.
     *
     * @param graph            Interference graph.
     * @param nodeId           The node that cannot be colored.
     * @param colorAssignment  Current partial color assignment (modified in place on success).
     * @param numRegisters     Total number of available registers.
     * @return Freed color index on success, or -1 if no rescue is possible.
     *
     * @complexity O(N^2 * (V + E)) where N = numRegisters
     */
    static int tryKempeRescue(
        const Graph<int>& graph,
        int nodeId,
        std::unordered_map<int, int>& colorAssignment,
        int numRegisters
    );
};

#endif // FREEALGORITHM_H
