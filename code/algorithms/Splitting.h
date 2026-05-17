/**
 * @file Splitting.h
 * @brief Declares Splitting — an iterative web-splitting register allocator.
 *
 * Attempts greedy graph coloring first. When coloring fails, it repeatedly:
 *   1. Selects the highest-degree web as the split candidate.
 *   2. Finds the optimal split point (minimum interference pressure).
 *   3. Creates two sub-webs and incrementally updates the interference graph.
 *   4. Retries graph coloring.
 *
 * This continues until coloring succeeds or the maximum number of splits is
 * reached, at which point all webs are assigned to memory.
 *
 */

#ifndef SPLITTING_H
#define SPLITTING_H

#include "../data_structures/Graph.h"
#include "../models/Web.h"
#include "GraphColoring.h"
#include <vector>
#include <unordered_map>

/**
 * @brief Register allocation via iterative web splitting.
 *
 * Attempts graph coloring first. If it fails, iteratively splits the
 * highest-degree web at the point of minimum interference and retries
 * coloring on the rebuilt interference graph, up to a maximum number
 * of splits.
 *
 * @complexity O(maxSplits * (V^2 + C))
 *   where V = number of webs and C = cost of GraphColoring::run().
 */
class Splitting {
public:
    /**
     * @brief Run register allocation with iterative web splitting.
     *
     * 1. Try GraphColoring::run(). If it succeeds, return immediately.
     * 2. Otherwise, for each round i in [1..maxSplits]:
     *    a. Select the web with the highest degree in the current
     *       interference graph.
     *    b. Split it at the point of minimum interference.
     *    c. Rebuild the interference graph and retry coloring.
     * 3. If coloring still fails after maxSplits rounds, return failure
     *    with all webs assigned to memory (-1).
     *
     * @param graph        Initial interference graph.
     * @param webs         Original list of webs.
     * @param numRegisters Number of available registers (K colors).
     * @param maxSplits    Maximum number of webs that can be split.
     * @return ColoringResult with success flag and web->register mapping.
     *
     * @complexity O(maxSplits * (V^2 + E)) where V = |webs|, E = |edges|.
     */
    static ColoringResult runWithSplitting(
        const Graph<int>& graph,
        const std::vector<Web>& webs,
        int numRegisters,
        int maxSplits
    );

private:
    /**
     * @brief Select the web with the highest degree in the interference graph.
     *
     * Iterates over all vertices and returns the ID of the one with the
     * most neighbours.
     *
     * @param graph Interference graph.
     * @param webs  Current list of webs (used to restrict candidates).
     * @return Web ID of the highest-degree web, or -1 if the graph is empty.
     *
     * @complexity O(V + E)
     */
    static int selectSplitCandidate(
        const Graph<int>& graph,
        const std::vector<Web>& webs
    );

    /**
     * @brief Find the optimal split point for a web.
     *
     * Iterates over the web's activeLines and, for each line, counts
     * how many other webs are simultaneously active. The cut is placed
     * before the line with the lowest interference count.
     *
     * @param web      The web to be split.
     * @param allWebs  All current webs (to compute interference counts).
     * @return Index into web.activeLines where the second sub-web begins.
     *         Returns -1 if the web cannot be split (fewer than 2 active lines).
     *
     * @complexity O(L * W) where L = |web.activeLines|, W = |allWebs|.
     */
    static int findSplitPoint(
        const Web& web,
        const std::vector<Web>& allWebs
    );
};

#endif // SPLITTING_H
