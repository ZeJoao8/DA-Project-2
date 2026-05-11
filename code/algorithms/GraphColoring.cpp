#include "GraphColoring.h"
#include <stack>
#include <unordered_set>
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

    // Build the active set: all webs not pre-spilled
    std::unordered_set<int> active;
    for (const auto& web : webs) {
        if (!spilled.count(web.id)) {
            active.insert(web.id);
        }
    }

    // Helper: compute degree of a node counting only active neighbors
    // Time Complexity: O(degree of node)
    auto getActiveDegree = [&](int id) -> int {
        auto* v = graph.findVertex(id);
        if (!v) return 0;
        int deg = 0;
        for (auto* edge : v->getAdj()) {
            int neighborId = edge->getDest()->getInfo();
            if (active.count(neighborId)) {
                deg++;
            }
        }
        return deg;
    };

    // -------------------------------------------------------
    // Phase 1 - Simplification: build the coloring stack
    // Repeatedly remove nodes with degree < N onto the stack.
    // If stuck (all active nodes have degree >= N), spill one.
    // Time Complexity: O(V^2) in the worst case
    // -------------------------------------------------------
    std::stack<int> colorStack;
    std::vector<int> forcedSpills; // nodes spilled during simplification

    while (!active.empty()) {
        bool removed = false;

        // Scan for any node with active degree < numRegisters
        for (int id : active) {
            if (getActiveDegree(id) < numRegisters) {
                colorStack.push(id);
                active.erase(id);
                removed = true;
                break; // Restart scan with updated degrees
            }
        }

        if (!removed && !active.empty()) {
            // All remaining nodes have degree >= N: must spill one
            std::vector<int> activeVec(active.begin(), active.end());
            int candidate = selectSpillCandidate(graph, {}, activeVec);

            if (candidate == -1) {
                // Safety fallback — should not happen
                result.success = false;
                break;
            }

            forcedSpills.push_back(candidate);
            active.erase(candidate);
            result.webToRegister[candidate] = -1;
        }
    }

    // If any node was force-spilled, the basic coloring was not fully successful
    if (!forcedSpills.empty()) {
        result.success = false;
    }

    // -------------------------------------------------------
    // Phase 2 - Coloring: pop stack and assign colors
    // Each node is guaranteed to have had degree < N when removed,
    // so a color is always available (unless forced spills exhausted colors).
    // Time Complexity: O(V * degree)
    // -------------------------------------------------------
    std::unordered_map<int, int> colorAssignment;

    while (!colorStack.empty()) {
        int id = colorStack.top();
        colorStack.pop();

        // Collect colors of already-assigned neighbors
        auto* v = graph.findVertex(id);
        std::vector<int> neighborColors;
        if (v) {
            for (auto* edge : v->getAdj()) {
                int neighborId = edge->getDest()->getInfo();
                auto it = colorAssignment.find(neighborId);
                if (it != colorAssignment.end() && it->second >= 0) {
                    neighborColors.push_back(it->second);
                }
            }
        }

        int color = assignColor(neighborColors, numRegisters);
        if (color == -1) {
            // Should not happen if Phase 1 worked correctly
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