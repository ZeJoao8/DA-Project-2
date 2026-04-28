#include "GraphBuilder.h"
#include <set>
#include <algorithm>
#include <iterator>

std::vector<Web> GraphBuilder::buildWebs(const std::unordered_map<std::string, VariableRanges>& parsedData) {
    std::vector<Web> allWebs;
    int nextWebId = 0;

    // Temporary structure to hold sets for easy merging and deduping
    struct TempWeb {
        std::set<int> activeLines;
        std::set<int> defLines;
        std::set<int> useLines;
    };

    for (const auto& [varName, varRanges] : parsedData) {
        std::vector<TempWeb> mergedRanges;

        // Process each individual range for the current variable
        for (const auto& range : varRanges.ranges) {
            TempWeb currentRange;
            for (const auto& point : range) {
                currentRange.activeLines.insert(point.line);
                if (point.isDef) {
                    currentRange.defLines.insert(point.line);
                }
                if (point.isEnd) {
                    currentRange.useLines.insert(point.line);
                }
            }

            // Greedy merge: find all existing merged ranges that overlap with the current range
            std::vector<TempWeb> nextMergedRanges;
            TempWeb combined = currentRange;

            for (const auto& existing : mergedRanges) {
                bool overlap = false;
                for (int line : currentRange.activeLines) {
                    if (existing.activeLines.count(line)) {
                        overlap = true;
                        break;
                    }
                }

                if (overlap) {
                    // Combine into the current range
                    combined.activeLines.insert(existing.activeLines.begin(), existing.activeLines.end());
                    combined.defLines.insert(existing.defLines.begin(), existing.defLines.end());
                    combined.useLines.insert(existing.useLines.begin(), existing.useLines.end());
                } else {
                    // No overlap, keep it separate
                    nextMergedRanges.push_back(existing);
                }
            }
            
            // Add the newly combined range
            nextMergedRanges.push_back(combined);
            mergedRanges = std::move(nextMergedRanges);
        }

        // Convert the final merged ranges into Web structs
        for (const auto& tw : mergedRanges) {
            Web web;
            web.id = nextWebId++;
            web.originalVariable = varName;
            
            web.activeLines.assign(tw.activeLines.begin(), tw.activeLines.end());
            web.defLines.assign(tw.defLines.begin(), tw.defLines.end());
            web.useLines.assign(tw.useLines.begin(), tw.useLines.end());
            
            // Vectors from std::set are already sorted, but we ensure sorting anyway
            std::sort(web.activeLines.begin(), web.activeLines.end());
            std::sort(web.defLines.begin(), web.defLines.end());
            std::sort(web.useLines.begin(), web.useLines.end());

            allWebs.push_back(web);
        }
    }

    return allWebs;
}

std::vector<std::pair<int, int>> GraphBuilder::calculateInterferences(const std::vector<Web>& webs) {
    std::vector<std::pair<int, int>> interferences;

    for (size_t i = 0; i < webs.size(); ++i) {
        for (size_t j = i + 1; j < webs.size(); ++j) {
            const Web& a = webs[i];
            const Web& b = webs[j];

            std::vector<int> sharedLines;
            std::set_intersection(a.activeLines.begin(), a.activeLines.end(),
                                  b.activeLines.begin(), b.activeLines.end(),
                                  std::back_inserter(sharedLines));

            // No common lines -> no interference
            if (sharedLines.empty()) {
                continue;
            }

            bool interferes = true;

            // Exception Rule: Two webs DO NOT interfere at line L if Web A ends at L 
            // and Web B begins at L (or vice-versa).
            // We check if ALL shared lines satisfy this exception rule (assuming they share no other lines
            // without this property).
            bool allExceptions = true;
            for (int L : sharedLines) {
                bool aUsesBDefs = std::binary_search(a.useLines.begin(), a.useLines.end(), L) &&
                                  std::binary_search(b.defLines.begin(), b.defLines.end(), L);
                
                bool bUsesADefs = std::binary_search(b.useLines.begin(), b.useLines.end(), L) &&
                                  std::binary_search(a.defLines.begin(), a.defLines.end(), L);

                if (!aUsesBDefs && !bUsesADefs) {
                    allExceptions = false;
                    break;
                }
            }

            // If every shared line is covered by the exception, they don't actually interfere
            if (allExceptions) {
                interferes = false;
            }

            if (interferes) {
                interferences.push_back({a.id, b.id});
            }
        }
    }

    return interferences;
}
