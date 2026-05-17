/**
 * @file main.cpp
 * @brief Entry point for the Compiler Register Allocation Tool.
 *
 * Supports two execution modes:
 *   - **Interactive mode** (no arguments): launches the Menu-driven UI.
 *   - **Batch mode** (`-b ranges registers output`): runs the full
 *     allocation pipeline non-interactively and writes results to a file.
 *
 */

#include <iostream>
#include <string>
#include "cli/BatchMode.h"
#include "cli/Menu.h"

/**
 * @brief Program entry point.
 *
 * Dispatches to interactive or batch mode depending on command-line arguments.
 *
 * @param argc Number of command-line arguments.
 * @param argv Argument vector:
 *   - `argc == 1`: interactive mode.
 *   - `argc == 5` and `argv[1] == "-b"`: batch mode with
 *     `argv[2]` = ranges file, `argv[3]` = registers file,
 *     `argv[4]` = output file.
 * @return 0 on success, 1 on invalid usage.
 *
 * @complexity O(1) for argument parsing; subsequent complexity
 *             depends on the chosen execution mode.
 */
int main(int argc, char* argv[]) {
    if (argc == 1) {
        // No arguments: Run interactive Menu
        Menu menu;
        menu.run();
    } else if (argc == 5) {
        // Exactly 5 arguments: Expected pattern `myProg -b ranges.txt registers.txt allocation.txt`
        std::string mode = argv[1];
        if (mode == "-b") {
            std::string rangesFile = argv[2];
            std::string registersFile = argv[3];
            std::string outputFile = argv[4];
            
            BatchMode::run(rangesFile, registersFile, outputFile);
        } else {
            std::cerr << "Error: Invalid mode specified." << std::endl;
            std::cerr << "Usage: " << argv[0] << " -b <ranges_file> <registers_file> <allocation_file>" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Error: Incorrect number of arguments." << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "  Interactive Mode: " << argv[0] << std::endl;
        std::cerr << "  Batch Mode:       " << argv[0] << " -b <ranges_file> <registers_file> <allocation_file>" << std::endl;
        return 1;
    }

    return 0;
}
