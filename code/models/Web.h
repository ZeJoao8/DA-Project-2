/**
 * @file Web.h
 * @brief Defines the Web struct used in register allocation.
 *
 * A **web** is the smallest unit of register allocation: a maximal
 * connected component of the def-use chains of a single variable.
 * Each web either receives a physical register or is spilled to memory.
 *
 */

#ifndef WEB_H
#define WEB_H

#include <string>
#include <vector>

/**
 * @brief Represents a variable web for register allocation.
 *
 * A web is derived from the live ranges of a single source variable.
 * Multiple webs may originate from the same variable when its live ranges
 * are disjoint and cannot be merged.
 */
struct Web {
    int id;                          ///< Unique numeric identifier for this web.
    std::string originalVariable;    ///< Name of the source variable this web was derived from.
    std::vector<int> activeLines;    ///< Sorted list of program lines where this web is live.
    std::vector<int> defLines;       ///< Sorted subset of activeLines where the variable is defined.
    std::vector<int> useLines;       ///< Sorted subset of activeLines where the variable is last used (range end).
};

#endif // WEB_H
