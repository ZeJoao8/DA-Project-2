/**
 * @file GraphBuilder.h
 * @brief Declares GraphBuilder — the core pipeline stage that converts
 *        variable live-range data into an interference graph.
 *
 * Pipeline:
 *   1. buildWebs()              — merge live-range segments into webs.
 *   2. calculateInterferences() — determine which pairs of webs interfere.
 *   3. buildInterferenceGraph() — construct the Graph<int> from interference pairs.
 *
 */

#ifndef GRAPHBUILDER_H
#define GRAPHBUILDER_H

#include "../io/DataLoader.h"
#include "../data_structures/Graph.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

#include "../models/Web.h"


class GraphBuilder {
public:
    /**
     * @brief Builds Webs from parsed variable ranges using a greedy merging algorithm.
     *
     * For each variable, the individual live-range segments in its VariableRanges
     * are merged greedily: any two segments that share at least one active line are
     * combined into a single Web. The result is a set of maximally disjoint webs
     * for each source variable.
     *
     * @param parsedData The data parsed from ranges.txt by DataLoader::loadRanges().
     * @return std::vector<Web> A list of globally unique Webs (unique IDs starting at 0).
     *
     * @pre @p parsedData must be fully populated and valid, containing no empty live ranges.
     *
     * @complexity O(V * R^2 * L) where V = number of variables,
     *             R = maximum number of range segments per variable, and
     *             L = maximum number of active lines per segment.
     */
    static std::vector<Web> buildWebs(const std::unordered_map<std::string, VariableRanges>& parsedData);

    /**
     * @brief Calculates interference edges between all pairs of Webs.
     *
     * Two webs **interfere** if they share at least one active program line,
     * unless all shared lines satisfy the *exception rule*: at every shared
     * line L, one web uses L as its last-use point while the other defines L
     * (i.e., the live ranges are adjacent, not overlapping, at L).
     *
     * @param webs The list of webs produced by buildWebs().
     * @return A list of (id_a, id_b) pairs representing interference edges.
     *
     * @pre @p webs must have ascending unique IDs starting from 0, and each web's
     *      `activeLines` must be sorted.
     *
     * @complexity O(W^2 * L) where W = number of webs and L = max number
     *             of shared active lines per pair (set intersection step).
     */
    static std::vector<std::pair<int, int>> calculateInterferences(const std::vector<Web>& webs);

    /**
     * @brief Builds an interference Graph<int> from a list of Webs.
     *
     * Each Web becomes a vertex identified by its `id`. For every pair
     * returned by calculateInterferences(), a bidirectional edge of weight 1
     * is added to the graph.
     *
     * @param webs The list of Webs to build the graph from.
     * @return Graph<int> where each vertex info equals the corresponding web ID.
     *
     * @pre The IDs of the given @p webs must be densely packed and strictly match the
     *      internal generation order for optimal lookup.
     *
     * @complexity O(W^2 * L + E) where W = number of webs, L = max shared
     *             lines per pair (for interference checks), and E = number
     *             of interference edges added.
     */
    static Graph<int> buildInterferenceGraph(const std::vector<Web>& webs);
};

#endif // GRAPHBUILDER_H
