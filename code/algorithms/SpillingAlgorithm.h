#ifndef SPILLINGALGORITHM_H
#define SPILLINGALGORITHM_H

#include "../data_structures/Graph.h"
#include "../models/Web.h"
#include "GraphColoring.h"
#include <unordered_map>
#include <vector>

/**
 * @brief Register allocation with advanced spilling heuristic (Chaitin).
 */
class SpillingAlgorithm {
public:
  /**
   * @brief Run register allocation with spilling.
   *
   * @param graph          Interference graph.
   * @param webs           Available webs.
   * @param numRegisters   Number of registers.
   * @return ColoringResult
   */
  static ColoringResult run(const Graph<int> &graph,
                            const std::vector<Web> &webs, int numRegisters,
                            int maxSpills);

private:
  /**
   * @brief Calculate the spill cost for a web.
   *
   * Cost is defined as (number of defs + number of uses).
   */
  static double calculateSpillCost(const Web &web);

  /**
   * @brief Select the best spill candidate using Chaitin's heuristic:
   * min(Cost/Degree).
   *
   * @param graph     Interference graph.
   * @param webs      List of all webs (for metadata/costs).
   * @param activeIds IDs of webs currently in the graph.
   * @return Web ID of candidate.
   */
  static int selectSpillCandidate(const Graph<int> &graph,
                                  const std::vector<Web> &webs,
                                  const std::vector<int> &activeIds);
};

#endif // SPILLINGALGORITHM_H
