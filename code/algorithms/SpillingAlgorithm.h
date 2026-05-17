/**
 * @file SpillingAlgorithm.h
 * @brief Declares SpillingAlgorithm — a Chaitin-style register allocator
 *        that explicitly spills webs to memory when necessary.
 *
 * Algorithm overview:
 *   - **Phase 0 (Init)**: compute active degrees; identify trivially colorable nodes.
 *   - **Phase 1 (Simplification)**: remove trivial nodes (degree < K) onto a stack;
 *     when none exist, select and remove a spill candidate using Chaitin's
 *     min(cost/degree) heuristic.
 *   - **Phase 2 (Coloring)**: pop nodes from the stack and assign the lowest
 *     free register color.
 *
 */

#ifndef SPILLINGALGORITHM_H
#define SPILLINGALGORITHM_H

#include "../data_structures/Graph.h"
#include "../models/Web.h"
#include "GraphColoring.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * @brief Register allocation with advanced spilling heuristic (Chaitin).
 */
class SpillingAlgorithm {
public:
  /**
   * @brief Run register allocation with spilling.
   *
   * Executes the three-phase Chaitin-style allocator. The process terminates
   * early (returning `result.success = false`) if more than `maxSpills` webs
   * would need to be spilled.
   *
   * @param graph          Interference graph (vertices keyed by web ID).
   * @param webs           All webs to allocate.
   * @param numRegisters   Number of available physical registers (K colors).
   * @param maxSpills      Maximum number of webs that may be spilled.
   *                       Allocation fails immediately if this limit is exceeded.
   * @return ColoringResult with `success`, `numColorsUsed`, `webToRegister`
   *         (register index or -1 for spilled webs), and `spilledWebs`.
   *
   * @complexity O(V + E) where V = number of webs and E = number of
   *             interference edges (each node and edge processed O(1) times).
   */
  static ColoringResult run(const Graph<int> &graph,
                            const std::vector<Web> &webs, int numRegisters,
                            int maxSpills);

private:
  /**
   * @brief Calculate the spill cost for a web.
   *
   * Cost is defined as `(|defLines| + |useLines|)`. Webs that are defined
   * and used frequently are costly to spill because each spill generates
   * additional load/store memory operations.
   *
   * @param web The web whose spill cost is computed.
   * @return Spill cost as a double (always ≥ 0).
   *
   * @complexity O(1)
   */
  static double calculateSpillCost(const Web &web);

  /**
   * @brief Select the best spill candidate using Chaitin's heuristic.
   *
   * Evaluates all currently active nodes and returns the one that minimises:
   * @code
   *              Spill Cost
   *   metric = ---------------
   *             Active Degree
   * @endcode
   * A node with a low metric is cheap to remove (low cost, many constraints
   * resolved). Isolated nodes (degree 0) use an effective degree of 1 to
   * avoid division by zero.
   *
   * @param active         Set of currently active web IDs.
   * @param webMap         Map from web ID to its Web metadata pointer.
   * @param currentDegrees Map from web ID to its current active-graph degree.
   * @return ID of the best spill candidate, or -1 if @p active is empty.
   *
   * @complexity O(V) where V = |active|.
   */
  static int selectSpillCandidate(const std::unordered_set<int> &active,
                                  const std::unordered_map<int, const Web *> &webMap,
                                  const std::unordered_map<int, int> &currentDegrees);
};

#endif // SPILLINGALGORITHM_H
