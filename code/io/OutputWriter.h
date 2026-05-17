/**
 * @file OutputWriter.h
 * @brief Declares OutputWriter — the I/O layer for writing allocation results.
 *
 * Produces the canonical output format consumed by the project's grader:
 * web listings, total register count, and per-web register or memory assignments.
 *
 */

#ifndef OUTPUTWRITER_H
#define OUTPUTWRITER_H

#include "../models/Web.h"
#include "../algorithms/GraphColoring.h"
#include <string>
#include <vector>

class OutputWriter {
public:
    /**
     * @brief Writes the register allocation result to a plain-text file.
     *
     * Output format:
     * @code
     *   webs: <N>
     *   web0: 1+, 2, 3-
     *   ...
     *   registers: <K>
     *   r0: web0
     *   r1: web2
     *   M: web1        # spilled to memory
     * @endcode
     *
     * @param filePath              Destination file path.
     * @param webs                  List of webs written in order of their IDs.
     * @param result                ColoringResult produced by an allocation algorithm.
     * @param numAvailableRegisters Total registers the allocator was given.
     * @return `true` if the file was created/written successfully; `false` otherwise.
     *
     * @complexity O(W * L) where W = number of webs and L = max active lines
     *             per web (due to sorting and annotation of each line).
     */
    static bool writeResult(const std::string& filePath,
                            const std::vector<Web>& webs,
                            const ColoringResult& result,
                            int numAvailableRegisters);
};

#endif // OUTPUTWRITER_H
