/**
 * @file SpillingAlgorithm.cpp
 * @brief Implements Chaitin-style register allocation with explicit spilling.
 *
 * Three phases: initialization, simplification/spill-selection, and coloring.
 * See SpillingAlgorithm.h for the full algorithm description.
 *
 */

#include "SpillingAlgorithm.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stack>
#include <unordered_set>

ColoringResult SpillingAlgorithm::run(const Graph<int> &graph,
                                      const std::vector<Web> &webs,
                                      int numRegisters, int maxSpills) {
  ColoringResult result;
  result.success = true;
  result.numColorsUsed = 0;

  // ------------------------------------------------------------------
  // Build auxiliary structures used throughout the allocation process.
  // ------------------------------------------------------------------

  std::unordered_map<int, const Web *>
      webMap; // Maps each web ID to its corresponding Web object.
  for (const auto &w : webs) {
    webMap[w.id] = &w;
    result.webToRegister[w.id] = -1;
  }

  std::unordered_set<int>
      active; // Tracks all currently active nodes in the interference graph.
  for (const auto &web : webs)
    active.insert(web.id);

  // ------------------------------------------------------------------
  // Phase 0 - Initialization: O(V + E)
  // ------------------------------------------------------------------
  // Compute the initial degree of every active node.
  // Nodes with degree strictly less than K (number of registers) are considered
  // trivially colorable and are immediately added to the simplification list.
  // ------------------------------------------------------------------
  std::unordered_map<int, int> currentDegrees;
  std::vector<int> trivialNodes;

  for (int id : active) {
    int deg = 0;
    auto *v = graph.findVertex(id);
    if (v) {
      for (auto *edge : v->getAdj()) {
        if (active.count(edge->getDest()->getInfo())) {
          deg++;
        }
      }
    }
    currentDegrees[id] = deg;

    // Nodes with degree < K are guaranteed to be colorable.
    if (deg < numRegisters) {
      trivialNodes.push_back(id);
    }
  }

  std::stack<int> colorStack;
  std::vector<int> forcedSpills;

  // ------------------------------------------------------------------
  // Phase 1 - Simplification and Spill Selection: O(V + E)
  // ------------------------------------------------------------------
  // Repeatedly:
  //   1. Remove trivial nodes.
  //   2. If none exist, select a spill candidate.
  //
  // Removed nodes are pushed onto a stack so they can later be
  // reinserted in reverse order during coloring.
  // ------------------------------------------------------------------

  while (!active.empty()) {

    // --------------------------------------------------------------
    // Case 1: A trivially colorable node exists.
    // --------------------------------------------------------------
    if (!trivialNodes.empty()) {
      // Extract a trivial node from the list
      int id = trivialNodes.back();
      trivialNodes.pop_back();

      // It might have already been removed due to an earlier Spill
      if (!active.count(id))
        continue;

      // Push the node onto the color stack, maintaining the simplification
      // order
      colorStack.push(id);
      active.erase(id);

      // Decrement the degree of all its active neighbors
      auto *v = graph.findVertex(id);
      if (v) {
        for (auto *edge : v->getAdj()) {
          int neighborId = edge->getDest()->getInfo();
          if (active.count(neighborId)) {
            currentDegrees[neighborId]--;
            // If a neighbor's degree just dropped below K, it has become
            // trivial. Push it to the list.
            if (currentDegrees[neighborId] == numRegisters - 1) {
              trivialNodes.push_back(neighborId);
            }
          }
        }
      }
    } else {
      // --------------------------------------------------------------
      // Case 2: No trivial nodes exist.
      // A spill is required to continue simplification.
      // --------------------------------------------------------------

      // Prevent excessive spilling.
      if ((int)forcedSpills.size() >= maxSpills) {
        std::cerr << "WARNING: Spill limit of " << maxSpills
                  << " exceeded. Allocation not possible.\n";
        result.success = false;
        return result;
      }

      // Select the best spill candidate using Chaitin's heuristic.
      int candidate = selectSpillCandidate(active, webMap, currentDegrees);

      if (candidate == -1 || !webMap.count(candidate)) {
        result.success = false;
        return result;
      }

      forcedSpills.push_back(candidate);
      active.erase(candidate);
      result.webToRegister[candidate] = -1;

      std::cout << "  Spilling web " << candidate << " ["
                << webMap.at(candidate)->originalVariable << "] to memory.\n";

      // ----------------------------------------------------------
      // Removing a spilled node also decreases the degree of all
      // its active neighbors.
      // ----------------------------------------------------------
      auto *v = graph.findVertex(candidate);
      if (v) {
        for (auto *edge : v->getAdj()) {
          int neighborId = edge->getDest()->getInfo();
          if (active.count(neighborId)) {
            currentDegrees[neighborId]--;
            // If the neighbor of the spilled node just became trivial, add it
            // to the list.
            if (currentDegrees[neighborId] == numRegisters - 1) {
              trivialNodes.push_back(neighborId);
            }
          }
        }
      }
    }
  }

  // ------------------------------------------------------------------
  // Phase 2 - Graph Coloring
  // ------------------------------------------------------------------
  // Nodes are reinserted in reverse order of removal.
  // Each node receives the first available register not already
  // used by one of its adjacent neighbors.
  // ------------------------------------------------------------------
  std::unordered_map<int, int> colorAssignment;

  while (!colorStack.empty()) {

    // Retrieve the next node to color.
    int id = colorStack.top();
    colorStack.pop();

    auto *v = graph.findVertex(id);

    // Tracks which colors are already used by adjacent nodes.
    std::unordered_set<int> usedColors;

    if (v) {
      for (auto *edge : v->getAdj()) {
        int neighborId = edge->getDest()->getInfo();

        // Check whether the neighbor has already been assigned a color.
        auto it = colorAssignment.find(neighborId);
        if (it != colorAssignment.end() && it->second >= 0)
          usedColors.insert(it->second);
      }
    }

    // --------------------------------------------------------------
    // Select the first available register.
    // --------------------------------------------------------------
    int color = -1;
    for (int c = 0; c < numRegisters; c++) {
      if (!usedColors.count(c)) {
        color = c;
        break;
      }
    }

    if (color == -1) {
      // This state is theoretically unreachable if Phase 1 was correctly
      // executed as nodes are only placed onto the stack if they are guaranteed
      // to find a color.
      std::cerr << "ERROR: Could not assign color to web " << id << "\n";
      result.success = false;
      return result;
    }

    // Assign the selected register to the current node.
    colorAssignment[id] = color;

    // Track the highest register index used.
    result.numColorsUsed = std::max(result.numColorsUsed, color + 1);
  }

  // ------------------------------------------------------------------
  // Store final register assignments.
  // ------------------------------------------------------------------
  for (const auto &[id, color] : colorAssignment)
    result.webToRegister[id] = color;

  result.spilledWebs = forcedSpills;
  return result;
}

// ------------------------------------------------------------------
// Calculates the spill cost of a web.
//
// Spill cost is estimated using the total number of definitions
// and uses associated with the web.
//
// Frequently used webs are more expensive to spill because they
// generate more memory load/store operations.
// ------------------------------------------------------------------

double SpillingAlgorithm::calculateSpillCost(const Web &web) {
  return static_cast<double>(web.defLines.size() + web.useLines.size());
}

// ------------------------------------------------------------------
// Selects the best spill candidate using Chaitin's heuristic:
//
//                Spill Cost
//    Metric = -----------------
//               Active Degree
//
//   - Spill Cost:
//       Estimated penalty of moving the web to memory.
//
//   - Active Degree:
//       Number of current interferences in the active graph.
//
// Intuitively, we prefer spilling nodes with:
//   - low spill cost;
//   - high degree.
//
// This minimizes memory overhead while removing many
// interference constraints from the graph.
// ------------------------------------------------------------------

int SpillingAlgorithm::selectSpillCandidate(
    const std::unordered_set<int> &active,
    const std::unordered_map<int, const Web *> &webMap,
    const std::unordered_map<int, int> &currentDegrees) {

  int bestCandidate = -1;
  double minMetric = std::numeric_limits<double>::max();

  // Iterate through all active nodes and select the one
  // with the lowest spill heuristic value (cost / degree).
  for (int id : active) {
    auto it = webMap.find(id);
    if (it == webMap.end())
      continue;

    int degree = currentDegrees.at(id);
    // Prevent division by zero for isolated nodes.
    // A degree of 0 is treated as 1 so that the
    // metric depends only on spill cost.
    if (degree == 0)
      degree = 1;

    double cost = calculateSpillCost(*it->second);
    double metric = cost / degree;

    // Greedily keep the node with the minimum metric.
    if (metric < minMetric) {
      minMetric = metric;
      bestCandidate = id;
    }
  }

  return bestCandidate;
}