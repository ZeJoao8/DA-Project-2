#include "FreeAlgorithm.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <stack>
#include <unordered_set>

// ---------------------------------------------------------------------------
// Phase 1 helper: builds the simplification stack exactly like GraphColoring
// ---------------------------------------------------------------------------
ColoringResult FreeAlgorithm::run(
    const Graph<int>& graph,
    const std::vector<Web>& webs,
    int numRegisters)
{
    ColoringResult result;
    result.success = true;
    result.numColorsUsed = 0;

    // Build active set from all webs
    std::unordered_set<int> active;
    for (const auto& web : webs) {
        active.insert(web.id);
    }

    // Helper: count active-only degree for a node
    auto getActiveDegree = [&](int id) -> int {
        auto* v = graph.findVertex(id);
        if (!v) return 0;
        int deg = 0;
        for (auto* edge : v->getAdj()) {
            if (active.count(edge->getDest()->getInfo()))
                deg++;
        }
        return deg;
    };

    // -----------------------------------------------------------------------
    // Phase 1 – Simplification: same as GraphColoring::run()
    // Repeatedly remove nodes with degree < N onto the stack.
    // If stuck, spill the highest-degree node (same heuristic as basic).
    // Time Complexity: O(V^2) worst case
    // -----------------------------------------------------------------------
    std::stack<int> colorStack;
    std::vector<int> forcedSpills;

    while (!active.empty()) {
        bool removed = false;

        for (int id : active) {
            if (getActiveDegree(id) < numRegisters) {
                colorStack.push(id);
                active.erase(id);
                removed = true;
                break;
            }
        }

        if (!removed && !active.empty()) {
            // Optimistic coloring: push candidate onto stack anyway, rather than spilling immediately
            int bestCandidate = -1;
            int bestDegree = -1;
            for (int id : active) {
                int deg = getActiveDegree(id);
                if (deg > bestDegree) {
                    bestDegree = deg;
                    bestCandidate = id;
                }
            }

            if (bestCandidate == -1) {
                result.success = false;
                break;
            }

            colorStack.push(bestCandidate);
            active.erase(bestCandidate);
            removed = true;
        }
    }

    // -----------------------------------------------------------------------
    // Phase 2 – Coloring with Kempe Chain rescue
    // Pop nodes from the stack and assign the lowest free color.
    // If no free color exists, attempt a Kempe Chain swap before spilling.
    // Time Complexity: O(V * N^2 * (V + E)) in the worst case
    // -----------------------------------------------------------------------
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

        // Try simple color assignment first
        std::unordered_set<int> usedColors(neighborColors.begin(), neighborColors.end());
        int color = -1;
        for (int c = 0; c < numRegisters; c++) {
            if (!usedColors.count(c)) {
                color = c;
                break;
            }
        }

        if (color == -1) {
            // All colors taken — attempt Kempe Chain rescue
            std::cout << "  [Kempe] Node " << id
                      << " stuck (all " << numRegisters
                      << " registers taken). Attempting rescue...\n";

            color = tryKempeRescue(graph, id, colorAssignment, numRegisters);

            if (color != -1) {
                std::cout << "  [Kempe] Rescue succeeded! Freed register "
                          << color << " for node " << id << ".\n";
            } else {
                std::cout << "  [Kempe] Rescue failed. Spilling node " << id << ".\n";
                result.success = false;
                colorAssignment[id] = -1;
                result.spilledWebs.push_back(id);
                continue;
            }
        }

        colorAssignment[id] = color;
        result.numColorsUsed = std::max(result.numColorsUsed, color + 1);
    }

    // Merge colorAssignment into result
    for (const auto& [id, color] : colorAssignment) {
        result.webToRegister[id] = color;
    }

    return result;
}

// ---------------------------------------------------------------------------
// buildKempeChain
// BFS from startId, traversing only c1- or c2-colored nodes in the graph.
// ---------------------------------------------------------------------------
std::unordered_set<int> FreeAlgorithm::buildKempeChain(
    const Graph<int>& graph,
    const std::unordered_map<int, int>& colorAssignment,
    int startId,
    int c1,
    int c2)
{
    std::unordered_set<int> chain;
    std::queue<int> bfsQueue;

    bfsQueue.push(startId);
    chain.insert(startId);

    while (!bfsQueue.empty()) {
        int current = bfsQueue.front();
        bfsQueue.pop();

        auto* v = graph.findVertex(current);
        if (!v) continue;

        for (auto* edge : v->getAdj()) {
            int neighborId = edge->getDest()->getInfo();
            if (chain.count(neighborId)) continue; // already visited

            auto it = colorAssignment.find(neighborId);
            if (it == colorAssignment.end()) continue; // not yet colored

            int neighborColor = it->second;
            if (neighborColor == c1 || neighborColor == c2) {
                chain.insert(neighborId);
                bfsQueue.push(neighborId);
            }
        }
    }

    return chain;
}

// ---------------------------------------------------------------------------
// tryKempeRescue
// For each color c used by a neighbor of nodeId, try to swap it with every
// other color c2 used by a different neighbor, validating that the chain
// starting at the c-neighbor does NOT contain any c2-neighbor of nodeId.
// ---------------------------------------------------------------------------
int FreeAlgorithm::tryKempeRescue(
    const Graph<int>& graph,
    int nodeId,
    std::unordered_map<int, int>& colorAssignment,
    int numRegisters)
{
    auto* vNode = graph.findVertex(nodeId);
    if (!vNode) return -1;

    // Build neighbor -> color map (only already-colored neighbors)
    // and group neighbors by color
    std::unordered_map<int, std::vector<int>> colorToNeighbors; // color -> list of neighbor IDs
    for (auto* edge : vNode->getAdj()) {
        int neighborId = edge->getDest()->getInfo();
        auto it = colorAssignment.find(neighborId);
        if (it != colorAssignment.end() && it->second >= 0) {
            colorToNeighbors[it->second].push_back(neighborId);
        }
    }

    // Collect all colors that are actually saturating nodeId
    std::vector<int> usedColorsList;
    for (const auto& [color, _] : colorToNeighbors) {
        usedColorsList.push_back(color);
    }

    // Try to free color c by swapping c <-> c2
    for (int c : usedColorsList) {
        const std::vector<int>& cNeighbors = colorToNeighbors.at(c);

        for (int c2 : usedColorsList) {
            if (c2 == c) continue;

            const std::vector<int>& c2Neighbors = colorToNeighbors.at(c2);

            // Try each c-colored neighbor of nodeId as the chain start
            for (int nodeA : cNeighbors) {
                std::unordered_set<int> chain =
                    buildKempeChain(graph, colorAssignment, nodeA, c, c2);

                // Validate: no c2-neighbor of nodeId may be in the chain
                bool valid = true;
                for (int nodeB : c2Neighbors) {
                    if (chain.count(nodeB)) {
                        valid = false;
                        break;
                    }
                }

                if (valid) {
                    // Execute the swap: c <-> c2 for all nodes in the chain
                    for (int chainNode : chain) {
                        int& assignedColor = colorAssignment.at(chainNode);
                        assignedColor = (assignedColor == c) ? c2 : c;
                    }
                    // Color c is now free for nodeId
                    return c;
                }
            }
        }
    }

    return -1; // No rescue possible
}
