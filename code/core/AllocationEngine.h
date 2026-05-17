#ifndef ALLOCATIONENGINE_H
#define ALLOCATIONENGINE_H

#include "../models/RegisterConfig.h"
#include "../models/Web.h"
#include "../data_structures/Graph.h"
#include <string>
#include <vector>

namespace AllocationEngine {
    /**
     * @brief Centralized algorithm dispatching, success checking, and output writing.
     * 
     * Unifies the interactive Menu and BatchMode allocations.
     * 
     * @param interferenceGraph   The interference graph.
     * @param webs                 The list of webs.
     * @param registerConfig       The parsed registers/algorithms configuration.
     * @param outputFile           The path to write the output result.
     */
    void runAllocation(
        const Graph<int>& interferenceGraph,
        const std::vector<Web>& webs,
        const RegisterConfig& registerConfig,
        const std::string& outputFile
    );
}

#endif // ALLOCATIONENGINE_H
