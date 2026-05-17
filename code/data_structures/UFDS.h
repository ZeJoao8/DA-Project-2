/**
 * @file UFDS.h
 * @brief Union-Find Disjoint Set (UFDS) with path compression and union by rank.
 *
 * Originally created for Kruskal's MST algorithm; included in the project
 * as part of the standard Graph data-structure library.
 *
 * @author Gonçalo Leão
 * @date 25/01/2022
 */

#ifndef DA_TP_CLASSES_UFDS
#define DA_TP_CLASSES_UFDS

#include <vector>

/**
 * @brief Union-Find Disjoint Set structure with path compression and
 *        union by rank.
 *
 * Supports near-constant-time set membership queries and merges.
 * Uses path compression in findSet() and union-by-rank in linkSets()
 * to achieve amortised O(α(N)) per operation (where α is the inverse
 * Ackermann function — effectively O(1) in practice).
 */
class UFDS {
public:
    /**
     * @brief Initialises N disjoint singleton sets {0}, {1}, ..., {N-1}.
     *
     * @param N Total number of elements.
     * @complexity O(N)
     */
    UFDS(unsigned int N);

    /**
     * @brief Finds the representative (root) of the set containing element @p i.
     *
     * Applies **path compression**: after the call, all nodes on the path
     * from @p i to the root point directly to the root.
     *
     * @param i Element index (0 ≤ i < N).
     * @return The root element of the set containing @p i.
     * @complexity O(α(N)) amortised.
     */
    unsigned long findSet(unsigned int i);

    /**
     * @brief Returns true if elements @p i and @p j belong to the same set.
     *
     * @param i First element index.
     * @param j Second element index.
     * @return `true` if findSet(i) == findSet(j).
     * @complexity O(α(N)) amortised.
     */
    bool isSameSet(unsigned int i, unsigned int j);

    /**
     * @brief Merges the sets containing @p i and @p j (union by rank).
     *
     * If i and j are already in the same set, this is a no-op.
     * The root with the **lower rank** becomes a child of the root with
     * the higher rank. If ranks are equal, @p j's root becomes the new
     * root and its rank is incremented.
     *
     * @param i First element index.
     * @param j Second element index.
     * @complexity O(α(N)) amortised.
     */
    void linkSets(unsigned int i, unsigned int j);

private:
    std::vector<unsigned int> path; ///< path[i] = parent of node i (itself if root).
    std::vector<unsigned int> rank; ///< rank[i] = upper bound on the height of the subtree rooted at i.
};


#endif //DA_TP_CLASSES_UFDS
