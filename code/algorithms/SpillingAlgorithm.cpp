#include "SpillingAlgorithm.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stack>
#include <unordered_set>

ColoringResult
SpillingAlgorithm::run(const Graph<int> &graph, 
                       const std::vector<Web> &webs,
                       int numRegisters,
                       int maxSpills) 
{
  ColoringResult result;
  result.success = true;
  result.numColorsUsed = 0;

  std::unordered_map<int, const Web *> webMap;
  for (const auto &w : webs) {
    webMap[w.id] = &w;
    result.webToRegister[w.id] = -1;
  }

  std::unordered_set<int> active;
  for (const auto &web : webs)
    active.insert(web.id);

  auto getActiveDegree = [&](int id) -> int {
    auto *v = graph.findVertex(id);
    if (!v)
      return 0;
    int deg = 0;
    for (auto *edge : v->getAdj())
      if (active.count(edge->getDest()->getInfo()))
        deg++;
    return deg;
  };

  std::stack<int> colorStack;
  std::vector<int> forcedSpills;

  // Phase 1 - Simplification
  while (!active.empty()) {
    bool removed = false;

    for (auto it = active.begin(); it != active.end();) {
      if (getActiveDegree(*it) < numRegisters) {
        colorStack.push(*it);
        it = active.erase(it);
        removed = true;
        break;
      } else {
        ++it;
      }
    }

    if (!removed && !active.empty()) {
      // Verificar limite K
      if ((int)forcedSpills.size() >= maxSpills) {
        std::cerr << "WARNING: Spill limit of " << maxSpills
                  << " exceeded. Allocation not possible.\n";
        result.success = false;
        return result; // success = false
      }

      std::vector<int> activeVec(active.begin(), active.end());
      int candidate = selectSpillCandidate(graph, webs, activeVec);

      if (candidate == -1 || !webMap.count(candidate)) {
        return result; // failure
      }

      forcedSpills.push_back(candidate);
      active.erase(candidate);
      result.webToRegister[candidate] = -1;

      std::cout << "  Spilling web " << candidate << " ["
                << webMap.at(candidate)->originalVariable
                << "] to memory.\n";
    }
  }

  // Phase 2 - Coloring
  std::unordered_map<int, int> colorAssignment;

  while (!colorStack.empty()) {
    int id = colorStack.top();
    colorStack.pop();

    auto *v = graph.findVertex(id);
    std::unordered_set<int> usedColors;

    if (v) {
      for (auto *edge : v->getAdj()) {
        int neighborId = edge->getDest()->getInfo();
        auto it = colorAssignment.find(neighborId);
        if (it != colorAssignment.end() && it->second >= 0)
          usedColors.insert(it->second);
      }
    }

    int color = -1;
    for (int c = 0; c < numRegisters; c++) {
      if (!usedColors.count(c)) {
        color = c;
        break;
      }
    }

    if (color == -1) {
      // Não devia acontecer se a fase 1 correu bem
      std::cerr << "ERROR: Could not assign color to web " << id << "\n";
      return result; // success = false
    }

    colorAssignment[id] = color;
    result.numColorsUsed = std::max(result.numColorsUsed, color + 1);
  }

  for (const auto &[id, color] : colorAssignment)
    result.webToRegister[id] = color;

  result.spilledWebs = forcedSpills;
  return result;
}

double SpillingAlgorithm::calculateSpillCost(const Web &web) {
  return static_cast<double>(web.defLines.size() + web.useLines.size());
}

int SpillingAlgorithm::selectSpillCandidate(const Graph<int> &graph,
                                            const std::vector<Web> &webs,
                                            const std::vector<int> &activeIds) {
  std::unordered_map<int, const Web *> activeWebs;
  std::unordered_set<int> activeSet(activeIds.begin(), activeIds.end());
  for (const auto &w : webs)
    if (activeSet.count(w.id))
      activeWebs[w.id] = &w;

  int bestCandidate = -1;
  double minMetric = std::numeric_limits<double>::max();

  for (int id : activeIds) {
    auto *v = graph.findVertex(id);
    if (!v)
      continue;

    int degree = 0;
    for (auto *edge : v->getAdj())
      if (activeSet.count(edge->getDest()->getInfo()))
        degree++;
    if (degree == 0)
      degree = 1;
    auto it = activeWebs.find(id);
    if (it == activeWebs.end()) continue;
    double cost = calculateSpillCost(*it->second);
    double metric = cost / degree;

    if (metric < minMetric) {
      minMetric = metric;
      bestCandidate = id;
    }
  }

  return bestCandidate;
}