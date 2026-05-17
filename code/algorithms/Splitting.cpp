#include "Splitting.h"
#include "../core/GraphBuilder.h"
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <limits>

namespace {
    bool interferes(const Web& a, const Web& b) {
        std::vector<int> sharedLines;
        std::set_intersection(a.activeLines.begin(), a.activeLines.end(),
                              b.activeLines.begin(), b.activeLines.end(),
                              std::back_inserter(sharedLines));

        // No common lines -> no interference
        if (sharedLines.empty()) {
            return false;
        }

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

        return !allExceptions;
    }
}

ColoringResult Splitting::runWithSplitting(
    const Graph<int>& graph,
    const std::vector<Web>& webs,
    int numRegisters,
    int maxSplits)
{
    // ------------------------------------------------------------------
    // Iteratively split webs and retry coloring.
    // Each round: O(V+E) candidate selection + O(L*W) split point
    //           + O(V^2) graph rebuild + O(V^2) coloring
    // Total: O(maxSplits * (V^2 + L*W))
    // ------------------------------------------------------------------
    ColoringResult result;
    std::vector<Web> currentWebs = webs;
    Graph<int> currentGraph = GraphBuilder::buildInterferenceGraph(currentWebs);

    // Compute next available web ID (continue from highest existing + 1)
    int nextWebId = 0;
    for (const auto& w : currentWebs) {
        nextWebId = std::max(nextWebId, w.id + 1);
    }

    for (int round = 1; round <= maxSplits; ++round) {

        // (a) Select the web with the highest degree
        int candidateId = selectSplitCandidate(currentGraph, currentWebs);
        if (candidateId == -1) {
            std::cerr << "  Splitting: No candidate found at round "
                      << round << ".\n";
            break;
        }

        // Find the Web object for the candidate
        const Web* candidateWeb = nullptr;
        size_t candidateIdx = 0;
        for (size_t i = 0; i < currentWebs.size(); ++i) {
            if (currentWebs[i].id == candidateId) {
                candidateWeb = &currentWebs[i];
                candidateIdx = i;
                break;
            }
        }
        if (!candidateWeb) {
            std::cerr << "  Splitting: Candidate web " << candidateId
                      << " not found in web list.\n";
            break;
        }

        // (b) Find the optimal split point
        int cutIdx = findSplitPoint(*candidateWeb, currentWebs);
        if (cutIdx <= 0) {
            // Cannot split this web (too few active lines)
            std::cout << "  Splitting: Web " << candidateId
                      << " cannot be split (too few active lines). Skipping.\n";
            continue;
        }

        // (c) Create two sub-webs from the split
        Web webA;
        webA.id = nextWebId++;
        webA.originalVariable = candidateWeb->originalVariable;

        Web webB;
        webB.id = nextWebId++;
        webB.originalVariable = candidateWeb->originalVariable;

        // web_a gets activeLines [0..cutIdx-1], web_b gets [cutIdx..end]
        webA.activeLines.assign(
            candidateWeb->activeLines.begin(),
            candidateWeb->activeLines.begin() + cutIdx);

        webB.activeLines.assign(
            candidateWeb->activeLines.begin() + cutIdx,
            candidateWeb->activeLines.end());

        // Inherit defLines and useLines only for the lines each sub-web contains
        std::unordered_set<int> linesA(webA.activeLines.begin(),
                                       webA.activeLines.end());
        std::unordered_set<int> linesB(webB.activeLines.begin(),
                                       webB.activeLines.end());

        for (int line : candidateWeb->defLines) {
            if (linesA.count(line)) webA.defLines.push_back(line);
            if (linesB.count(line)) webB.defLines.push_back(line);
        }
        for (int line : candidateWeb->useLines) {
            if (linesA.count(line)) webA.useLines.push_back(line);
            if (linesB.count(line)) webB.useLines.push_back(line);
        }

        // Ensure sorted order (activeLines already come sorted from the
        // original web; defLines and useLines are filtered in order)
        std::sort(webA.defLines.begin(), webA.defLines.end());
        std::sort(webA.useLines.begin(), webA.useLines.end());
        std::sort(webB.defLines.begin(), webB.defLines.end());
        std::sort(webB.useLines.begin(), webB.useLines.end());

        std::cout << "  Splitting round " << round << ": web "
                  << candidateId << " ["
                  << candidateWeb->originalVariable << "] -> web "
                  << webA.id << " (lines "
                  << webA.activeLines.front() << "-"
                  << webA.activeLines.back() << ") + web "
                  << webB.id << " (lines "
                  << webB.activeLines.front() << "-"
                  << webB.activeLines.back() << ")\n";

        // (d) Replace the original web with the two sub-webs
        currentWebs.erase(currentWebs.begin() + candidateIdx);
        currentWebs.push_back(webA);
        currentWebs.push_back(webB);

        // (e) Update interference graph incrementally instead of rebuilding from scratch
        currentGraph.removeVertex(candidateId);
        currentGraph.addVertex(webA.id);
        currentGraph.addVertex(webB.id);

        for (const auto& other : currentWebs) {
            if (other.id == webA.id || other.id == webB.id) continue;
            
            if (interferes(webA, other)) {
                currentGraph.addBidirectionalEdge(webA.id, other.id, 1.0);
            }
            if (interferes(webB, other)) {
                currentGraph.addBidirectionalEdge(webB.id, other.id, 1.0);
            }
        }
        if (interferes(webA, webB)) {
            currentGraph.addBidirectionalEdge(webA.id, webB.id, 1.0);
        }

        // (f) Try coloring again
        result = GraphColoring::run(currentGraph, currentWebs, numRegisters, {});
        if (result.success) {
            std::cout << "  Splitting: Coloring succeeded after "
                      << round << " split(s).\n";
            result.splitWebs = currentWebs;
            return result;
        }
    }

    // ------------------------------------------------------------------
    // Step 3: All splits exhausted — return failure with all webs in memory.
    // ------------------------------------------------------------------
    std::cerr << "  Splitting: Coloring failed after " << maxSplits
              << " split(s). Assigning all webs to memory.\n";

    result.success = false;
    result.numColorsUsed = 0;
    result.webToRegister.clear();
    result.spilledWebs.clear();

    for (const auto& w : currentWebs) {
        result.webToRegister[w.id] = -1;
        result.spilledWebs.push_back(w.id);
    }

    result.splitWebs = currentWebs;
    return result;
}

int Splitting::selectSplitCandidate(
    const Graph<int>& graph,
    const std::vector<Web>& webs)
{
    std::unordered_set<int> webIds;
    for (const auto& w : webs) {
        webIds.insert(w.id);
    }

    // Build a map from web ID to number of active lines for tie-breaking
    std::unordered_map<int, int> webSize;
    for (const auto& w : webs) {
        webSize[w.id] = static_cast<int>(w.activeLines.size());
    }

    int bestId = -1;
    int bestDegree = -1;
    int bestSize = -1;

    for (auto* v : graph.getVertexSet()) {
        int id = v->getInfo();
        if (!webIds.count(id)) continue;

        int deg = 0;
        for (auto* edge : v->getAdj()) {
            if (webIds.count(edge->getDest()->getInfo())) {
                deg++;
            }
        }

        int size = webSize.count(id) ? webSize[id] : 0;

        // Prefer higher degree; break ties by preferring longer webs
        if (deg > bestDegree || (deg == bestDegree && size > bestSize)) {
            bestDegree = deg;
            bestId = id;
            bestSize = size;
        }
    }

    return bestId;
}

int Splitting::findSplitPoint(
    const Web& web,
    const std::vector<Web>& allWebs)
{
    const auto& lines = web.activeLines;
    int n = static_cast<int>(lines.size());

    // Need at least 2 active lines to split
    if (n < 2) return -1;

    // For each candidate cut position (1..n-1), the split point is
    // "before lines[cutIdx]".  We pick the cut that minimises the
    // number of other webs active at that line (interference pressure).
    //
    // Time Complexity: O(n * W) where W = |allWebs|
    int bestCutIdx = 1;  // default: after the first line
    int minInterference = std::numeric_limits<int>::max();
    int midpoint = n / 2;

    for (int cutIdx = 1; cutIdx < n; ++cutIdx) {
        int line = lines[cutIdx];
        int interference = 0;

        // Count how many OTHER webs are active at this line
        for (const auto& other : allWebs) {
            if (other.id == web.id) continue;

            // other.activeLines is sorted, so binary search
            if (std::binary_search(other.activeLines.begin(),
                                   other.activeLines.end(),
                                   line)) {
                interference++;
            }
        }

        // Prefer lower interference; break ties by preferring the
        // cut closest to the midpoint (most balanced split)
        int curDist = cutIdx > midpoint ? cutIdx - midpoint : midpoint - cutIdx;
        int bestDist = bestCutIdx > midpoint ? bestCutIdx - midpoint : midpoint - bestCutIdx;
        if (interference < minInterference ||
            (interference == minInterference && curDist < bestDist)) {
            minInterference = interference;
            bestCutIdx = cutIdx;
        }
    }

    return bestCutIdx;
}
