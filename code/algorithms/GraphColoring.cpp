/**
 * @file GraphColoring.cpp
 * @brief Implements the greedy graph-coloring register allocator (GraphColoring).
 *
 * Provides the baseline allocation used as a first-pass attempt before any
 * fallback strategy (spilling, splitting, or Kempe rescue) is invoked.
 *
 */

#include "GraphColoring.h"
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iostream>

ColoringResult GraphColoring::run(
    const Graph<int>& graph,
    const std::vector<Web>& webs,
    int numRegisters,
    const std::vector<int>& spilledWebIds)
{
    ColoringResult result;
    result.success = true;
    result.numColorsUsed = 0;

    // Pre-mark externally spilled webs as memory (-1)
    std::unordered_set<int> spilled(spilledWebIds.begin(), spilledWebIds.end());
    for (int id : spilledWebIds) {
        result.webToRegister[id] = -1;
    }

    // Build the active set and vertex lookup map once
    std::unordered_set<int> active;
    std::unordered_map<int, Vertex<int>*> vertexMap;
    for (auto* v : graph.getVertexSet()) {
        vertexMap[v->getInfo()] = v;
    }

    for (const auto& web : webs) {
        if (!spilled.count(web.id)) {
            active.insert(web.id);
        }
    }

    // Dynamic degrees and trivial nodes initialization: O(V + E)
    std::unordered_map<int, int> currentDegrees;
    std::vector<int> trivialNodes;

    for (int id : active) {
        int deg = 0;
        auto it = vertexMap.find(id);
        if (it != vertexMap.end() && it->second != nullptr) {
            for (auto* edge : it->second->getAdj()) {
                if (active.count(edge->getDest()->getInfo())) {
                    deg++;
                }
            }
        }
        currentDegrees[id] = deg;

        if (deg < numRegisters) {
            trivialNodes.push_back(id);
        }
    }

    std::stack<int> colorStack;
    std::vector<int> forcedSpills; // nodes spilled during simplification

    // Simplification Loop: O(V + E)
    while (!active.empty()) {
        if (!trivialNodes.empty()) {
            int id = trivialNodes.back();
            trivialNodes.pop_back();

            if (!active.count(id)) {
                continue;
            }

            colorStack.push(id);
            active.erase(id);

            // Decrement degrees of active neighbors
            auto it = vertexMap.find(id);
            if (it != vertexMap.end() && it->second != nullptr) {
                for (auto* edge : it->second->getAdj()) {
                    int neighborId = edge->getDest()->getInfo();
                    if (active.count(neighborId)) {
                        currentDegrees[neighborId]--;
                        if (currentDegrees[neighborId] == numRegisters - 1) {
                            trivialNodes.push_back(neighborId);
                        }
                    }
                }
            }
        } else {
            // Need to spill a node: pick active node with highest degree
            int candidate = -1;
            int maxDeg = -1;
            for (int id : active) {
                int deg = currentDegrees[id];
                if (deg > maxDeg) {
                    maxDeg = deg;
                    candidate = id;
                }
            }

            if (candidate == -1) {
                result.success = false;
                break;
            }

            forcedSpills.push_back(candidate);
            active.erase(candidate);
            result.webToRegister[candidate] = -1;

            // Decrement degrees of active neighbors of the spilled node
            auto it = vertexMap.find(candidate);
            if (it != vertexMap.end() && it->second != nullptr) {
                for (auto* edge : it->second->getAdj()) {
                    int neighborId = edge->getDest()->getInfo();
                    if (active.count(neighborId)) {
                        currentDegrees[neighborId]--;
                        if (currentDegrees[neighborId] == numRegisters - 1) {
                            trivialNodes.push_back(neighborId);
                        }
                    }
                }
            }
        }
    }

    // If any node was force-spilled, the basic coloring was not fully successful
    if (!forcedSpills.empty()) {
        result.success = false;
    }

    // Phase 2 - Coloring: pop stack and assign colors
    // Use vertexMap for fast neighbor lookups!
    std::unordered_map<int, int> colorAssignment;

    while (!colorStack.empty()) {
        int id = colorStack.top();
        colorStack.pop();

        // Collect colors of already-assigned neighbors
        std::vector<int> neighborColors;
        auto itLookup = vertexMap.find(id);
        if (itLookup != vertexMap.end() && itLookup->second != nullptr) {
            for (auto* edge : itLookup->second->getAdj()) {
                int neighborId = edge->getDest()->getInfo();
                auto it = colorAssignment.find(neighborId);
                if (it != colorAssignment.end() && it->second >= 0) {
                    neighborColors.push_back(it->second);
                }
            }
        }

        int color = assignColor(neighborColors, numRegisters);
        if (color == -1) {
            result.success = false;
            colorAssignment[id] = -1;
        } else {
            colorAssignment[id] = color;
            result.numColorsUsed = std::max(result.numColorsUsed, color + 1);
        }
    }

    // Merge phase 2 results into final result
    for (const auto& [id, color] : colorAssignment) {
        result.webToRegister[id] = color;
    }

    return result;
}

int GraphColoring::selectSpillCandidate(
    const Graph<int>& graph,
    const std::vector<int>& excludedIds,
    const std::vector<int>& activeIds)
{
    std::unordered_set<int> excluded(excludedIds.begin(), excludedIds.end());
    std::unordered_set<int> activeSet(activeIds.begin(), activeIds.end());

    int bestCandidate = -1;
    int bestDegree = -1;

    for (auto* v : graph.getVertexSet()) {
        int id = v->getInfo();

        // Skip if excluded or not active
        if (excluded.count(id) || !activeSet.count(id)) continue;

        // Count active neighbors as the degree
        int deg = 0;
        for (auto* edge : v->getAdj()) {
            if (activeSet.count(edge->getDest()->getInfo())) {
                deg++;
            }
        }

        if (deg > bestDegree) {
            bestDegree = deg;
            bestCandidate = id;
        }
    }

    return bestCandidate;
}

int GraphColoring::assignColor(
    const std::vector<int>& neighborColors,
    int numColors)
{
    std::unordered_set<int> used(neighborColors.begin(), neighborColors.end());

    for (int color = 0; color < numColors; color++) {
        if (!used.count(color)) {
            return color;
        }
    }

    return -1; // All colors taken
}