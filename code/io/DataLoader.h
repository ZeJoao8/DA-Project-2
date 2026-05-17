/**
 * @file DataLoader.h
 * @brief Declares DataLoader — the I/O layer for reading program input files.
 *
 * DataLoader is responsible for:
 *   - Resolving relative filenames to absolute paths.
 *   - Parsing `registers.txt` into a RegisterConfig.
 *   - Parsing `ranges.txt` into a map of variable live-range data.
 *
 */

#ifndef DATALOADER_H
#define DATALOADER_H

#include <string>
#include <vector>
#include <unordered_map>

#include "../models/RegisterConfig.h"
#include "../models/LiveRange.h"


class DataLoader {
public:
    /**
     * @brief Resolves a short filename to a full path by checking common directories.
     *
     * Search order:
     *   1. The path as given (absolute or already correct relative).
     *   2. The current working directory.
     *   3. `../basic/<defaultDir>/<inputPath>`
     *   4. `../../basic/<defaultDir>/<inputPath>`
     *
     * @param inputPath  The filename or relative path to resolve.
     * @param defaultDir Sub-directory inside `basic/` where the file typically lives
     *                   (e.g., `"ranges"` or `"registers"`).
     * @return The first path that resolves to an existing file, or the
     *         `../basic/<defaultDir>/...` fallback if none is found.
     *
     * @complexity O(1) — at most four filesystem probes.
     */
    static std::string resolvePath(const std::string& inputPath, const std::string& defaultDir);

    /**
     * @brief Parses the `registers.txt` configuration file.
     *
     * Expected format (key: value pairs, one per line; comments start with `#`):
     * @code
     *   registers: 4
     *   algorithm: spilling, 3
     * @endcode
     *
     * @param filename Path to the configuration file.
     * @return RegisterConfig struct with numRegisters, algorithmName, and algorithmParam.
     * @throws std::runtime_error if the file cannot be opened, is missing required
     *         fields, or contains malformed values.
     *
     * @complexity O(L) where L is the number of non-comment lines in the file.
     */
    static RegisterConfig loadRegisters(const std::string& filename);

    /**
     * @brief Parses the `ranges.txt` file to extract variable live-range points.
     *
     * Each non-comment line has the form:
     * @code
     *   varName: line1+, line2, line3-, ...
     * @endcode
     * where `+` marks a definition and `-` marks the end of a live range.
     * Multiple lines for the same variable create separate range segments.
     *
     * @param filename Path to the ranges file.
     * @return An unordered_map from variable names to their VariableRanges data.
     * @throws std::runtime_error if the file cannot be opened or contains
     *         malformed lines.
     *
     * @complexity O(L * P) where L is the number of variable lines and
     *             P is the maximum number of program points per line.
     */
    static std::unordered_map<std::string, VariableRanges> loadRanges(const std::string& filename);
};

#endif // DATALOADER_H
