#ifndef GRAPHBUILDER_H
#define GRAPHBUILDER_H

#include "DataLoader.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

struct Web {
    int id;
    std::string originalVariable;
    std::vector<int> activeLines;
    std::vector<int> defLines;
    std::vector<int> useLines;
};

class GraphBuilder {
public:
    /**
     * @brief Builds Webs from parsed variable ranges using a greedy merging algorithm.
     * 
     * @param parsedData The data parsed from ranges.txt by DataLoader.
     * @return std::vector<Web> A list of globally unique Webs.
     */
    static std::vector<Web> buildWebs(const std::unordered_map<std::string, VariableRanges>& parsedData);

    /**
     * @brief Calculates interferences between Webs to build the Interference Graph.
     * 
     * Two webs interfere if they share an active line, EXCEPT if one web ends at that line 
     * and the other web is defined at that line.
     * 
     * @param webs The list of generated Webs.
     * @return std::vector<std::pair<int, int>> A list of edges representing interferences (Web ID pairs).
     */
    static std::vector<std::pair<int, int>> calculateInterferences(const std::vector<Web>& webs);
};

#endif // GRAPHBUILDER_H
