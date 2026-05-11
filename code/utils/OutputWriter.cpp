#include "OutputWriter.h"
#include <fstream>
#include <iostream>
#include <algorithm>

bool OutputWriter::writeResult(const std::string& filePath, 
                               const std::vector<Web>& webs, 
                               const ColoringResult& result,
                               int numAvailableRegisters) {
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        return false;
    }

    outFile << "# Total number of webs followed by the listing of the program points of each one\n";
    outFile << "# program points in each web are sorted in ascending order\n";
    outFile << "webs: " << webs.size() << "\n";

    for (const auto& web : webs) {
        outFile << "web" << web.id << ": ";
        
        // Copy and sort active lines just in case
        std::vector<int> sortedLines = web.activeLines;
        std::sort(sortedLines.begin(), sortedLines.end());

        for (size_t i = 0; i < sortedLines.size(); ++i) {
            int line = sortedLines[i];
            outFile << line;

            // Check if it's a definition point
            bool isDef = std::find(web.defLines.begin(), web.defLines.end(), line) != web.defLines.end();
            // Check if it's an end point (use point that terminates the range)
            bool isEnd = std::find(web.useLines.begin(), web.useLines.end(), line) != web.useLines.end();

            if (isDef) outFile << "+";
            if (isEnd) outFile << "-";

            if (i < sortedLines.size() - 1) {
                outFile << ",";
            }
        }
        outFile << "\n";
    }

    outFile << "# Total number of registers used, followed by assignment to webs\n";

    if (!result.success) {
        outFile << "registers: 0\n";
        for (const auto& web : webs) {
            outFile << "M: web" << web.id << "\n";
        }
    } else {
        outFile << "registers: " << result.numColorsUsed << "\n";
        
        // Output rX: webY for each allocated web
        // Note: The example shows r0: web0, r0: web1, etc.
        // We can iterate through registers first to group them as in Figure 11
        for (int r = 0; r < numAvailableRegisters; ++r) {
            for (const auto& web : webs) {
                auto it = result.webToRegister.find(web.id);
                if (it != result.webToRegister.end() && it->second == r) {
                    outFile << "r" << r << ": web" << web.id << "\n";
                }
            }
        }

        // Output M: webY for any spilled webs (if any, although basic allocation might just fail)
        for (const auto& web : webs) {
            auto it = result.webToRegister.find(web.id);
            if (it == result.webToRegister.end() || it->second == -1) {
                outFile << "M: web" << web.id << "\n";
            }
        }
    }

    outFile.close();
    return true;
}
