#include "FreeAlgorithm.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>

ColoringResult FreeAlgorithm::run(
    const Graph<int>& graph,
    const std::vector<Web>& webs,
    int numRegisters)
{
    ColoringResult result;
    result.success = true;
    result.numColorsUsed = 0;

    // Build active set and vertex lookup map once
    std::unordered_set<int> active;
    std::unordered_map<int, Vertex<int>*> vertexMap;
    for (auto* v : graph.getVertexSet()) {
        vertexMap[v->getInfo()] = v;
    }

    for (const auto& web : webs) {
        active.insert(web.id);
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

    // Simplification Phase: O(V + E)
    while (!active.empty()) {
        if (!trivialNodes.empty()) {
            int id = trivialNodes.back();
            trivialNodes.pop_back();

            if (!active.count(id)) {
                continue;
            }

            colorStack.push(id);
            active.erase(id);

            // Decrement active neighbor degrees
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
            // Optimistic coloring: push highest-degree node onto stack anyway
            int bestCandidate = -1;
            int bestDegree = -1;
            for (int id : active) {
                int deg = currentDegrees[id];
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

            // Decrement active neighbor degrees of the optimistically pushed node
            auto it = vertexMap.find(bestCandidate);
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

    // -----------------------------------------------------------------------
    // Phase 2 – Coloring with Kempe Chain rescue
    // Pop nodes from the stack and assign the lowest free color.
    // If no free color exists, attempt a Kempe Chain swap before spilling.
    // Use vertexMap for fast lookup of adjacent nodes.
    // Time Complexity: O(V * N^2 * (V + E)) in the worst case
    // -----------------------------------------------------------------------
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
