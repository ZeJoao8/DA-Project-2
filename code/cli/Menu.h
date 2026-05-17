/**
 * @file Menu.h
 * @brief Declares the Menu class — the interactive CLI for the register
 *        allocation tool.
 *
 * The Menu presents a numbered option list and drives the full
 * allocation pipeline step-by-step:
 *   1. Load ranges and registers files.
 *   2. Display loaded data.
 *   3. Build and display webs.
 *   4. Build and display the interference graph.
 *   5. Run register allocation and write results.
 *   0. Exit.
 *
 */

#ifndef MENU_H
#define MENU_H

#include "../io/DataLoader.h"
#include "../core/GraphBuilder.h"
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Interactive command-line interface for the Compiler Register
 *        Allocation Tool.
 *
 * Maintains all intermediate state (loaded data, built webs, interference
 * graph) across menu iterations so that expensive steps are not repeated.
 */
class Menu {
private:
    bool dataLoaded = false;          ///< True after a successful option-1 data load.
    RegisterConfig registerConfig;    ///< Algorithm configuration loaded from registers file.
    std::unordered_map<std::string, VariableRanges> rangesMap; ///< Raw parsed ranges per variable.
    std::vector<Web> webs;            ///< Webs constructed by GraphBuilder::buildWebs().
    Graph<int> interferenceGraph;     ///< Interference graph built by GraphBuilder::buildInterferenceGraph().
    std::string outputFilePath;       ///< Destination path for the allocation result file.
    bool graphBuilt = false;          ///< True once the interference graph has been constructed.

public:
    /**
     * @brief Constructs a Menu with default (empty) state.
     * @complexity O(1)
     */
    Menu();

    /**
     * @brief Enters the interactive menu loop.
     *
     * Continuously displays the main menu and dispatches the user's choice
     * until option 0 (Exit) is selected or EOF is reached.
     *
     * @complexity O(1) per iteration; individual options may be O(V^2) or higher.
     */
    void run();
};

#endif // MENU_H
