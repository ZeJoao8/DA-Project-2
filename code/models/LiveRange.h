/**
 * @file LiveRange.h
 * @brief Defines the data structures that describe live ranges of variables.
 *
 * These structs are populated by DataLoader from the ranges input file and
 * are the primary input to GraphBuilder::buildWebs().
 *
 */

#ifndef LIVERANGE_H
#define LIVERANGE_H

#include <string>
#include <vector>

/**
 * @brief Represents a single program point within a live range.
 *
 * Each point corresponds to one line in the source program where the
 * variable has an observable event (definition, intermediate use, or
 * last use that ends the live range).
 */
struct LiveRangePoint {
    int  line;    ///< Source program line number.
    bool isDef;   ///< True if the variable is **defined** at this line.
    bool isEnd;   ///< True if this is the **last use** of the variable (range end).
};

/**
 * @brief Holds all live-range segments for a single source variable.
 *
 * A variable may have multiple disconnected live-range segments (e.g., after
 * a branch that kills and re-defines it). Each segment is a contiguous
 * sequence of LiveRangePoint entries.
 */
struct VariableRanges {
    std::string varName;                            ///< Source variable name.
    std::vector<std::vector<LiveRangePoint>> ranges; ///< One inner vector per live-range segment.
};

#endif // LIVERANGE_H
