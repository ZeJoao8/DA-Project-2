#ifndef OUTPUTWRITER_H
#define OUTPUTWRITER_H

#include "../models/Web.h"
#include "../algorithms/GraphColoring.h"
#include <string>
#include <vector>

class OutputWriter {
public:
    /**
     * @brief Writes the register allocation result to a .txt file.
     * 
     * @param filePath              Path to the output file.
     * @param webs                  The list of webs used in allocation.
     * @param result                The coloring result (success status, assignments).
     * @param numAvailableRegisters The number of registers the tool tried to allocate.
     * @return true if file was written successfully, false otherwise.
     */
    static bool writeResult(const std::string& filePath, 
                            const std::vector<Web>& webs, 
                            const ColoringResult& result,
                            int numAvailableRegisters);
};

#endif // OUTPUTWRITER_H
