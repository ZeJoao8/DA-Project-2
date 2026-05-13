#include "Menu.h"
#include "../algorithms/GraphColoring.h"
#include "../algorithms/SpillingAlgorithm.h"
#include "../utils/DataLoader.h"
#include "../utils/OutputWriter.h"
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

Menu::Menu() {
  // Constructor
}

void Menu::run() {
  int choice = -1;

  while (choice != 0) {
    std::cout << "\n--- Compiler Register Allocation Tool ---\n";
    std::cout << "1. Load Data (ranges and registers files)\n";
    std::cout << "2. Display Loaded Data (ranges and registers)\n";
    std::cout << "3. Build and Display Webs\n";
    std::cout << "4. Build and Display Interference Graph\n";
    std::cout << "5. Run Basic Register Allocation\n";
    std::cout << "6. Run Allocation with Web Spilling\n";
    std::cout << "7. Run Allocation with Web Splitting\n";
    std::cout << "8. Run Free Algorithm\n";
    std::cout << "0. Exit\n";
    std::cout << "Enter your choice: ";

    if (!(std::cin >> choice)) {
      std::cerr << "Invalid input. Please enter a number.\n";
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      choice = -1;
      continue;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (choice) {

    // ── 1. Load Data ──────────────────────────────────────────
    case 1: {
      std::string rangesPath, registersPath;

      std::cout << "Enter ranges file path: ";
      std::getline(std::cin, rangesPath);

      std::cout << "Enter registers file path: ";
      std::getline(std::cin, registersPath);

      std::cout << "Enter output file path: ";
      std::getline(std::cin, outputFilePath);

      std::cout << "Loading data...\n";
      dataLoaded = false;
      rangesMap.clear();
      registerConfig = RegisterConfig();
      webs.clear();
      graphBuilt = false;

      try {
        rangesPath = DataLoader::resolvePath(rangesPath, "ranges");
        registersPath = DataLoader::resolvePath(registersPath, "registers");

        rangesMap = DataLoader::loadRanges(rangesPath);
        registerConfig = DataLoader::loadRegisters(registersPath);
        dataLoaded = true;
        std::cout << "Successfully loaded " << rangesMap.size()
                  << " variables and " << registerConfig.numRegisters
                  << " registers.\n";

        // Auto-generate webs
        webs = GraphBuilder::buildWebs(rangesMap);
        std::cout << "Generated " << webs.size() << " webs.\n";
      } catch (const std::exception &e) {
        std::cerr << "Error: Failed to load data. Returning to menu.\n";
      }
      break;
    }

    // ── 2. Display Loaded Data ────────────────────────────────
    case 2: {
      if (!dataLoaded) {
        std::cerr << "Error: No data loaded. Use option 1 first.\n";
        break;
      }
      std::cout << "\n--- Register Configuration ---\n";
      std::cout << "  Registers: " << registerConfig.numRegisters << "\n";
      std::cout << "  Algorithm: " << registerConfig.algorithmName << "\n";
      std::cout << "  Param:     " << registerConfig.algorithmParam << "\n";

      std::cout << "\n--- Variable Ranges ---\n";
      for (const auto &[name, varRanges] : rangesMap) {
        std::cout << "  Variable '" << name << "':\n";
        for (size_t i = 0; i < varRanges.ranges.size(); ++i) {
          std::cout << "    Segment " << i + 1 << ": ";
          for (const auto &point : varRanges.ranges[i]) {
            std::cout << "" << point.line;
            if (point.isDef)
              std::cout << "+,";
            else if (point.isEnd)
              std::cout << "-";
            else
              std::cout << ",";
          }
          std::cout << "\n";
        }
      }
      break;
    }

    // ── 3. Build and Display Webs ─────────────────────────────
    case 3: {
      if (!dataLoaded) {
        std::cerr << "Error: No data loaded. Use option 1 first.\n";
        break;
      }
      if (webs.empty()) {
        webs = GraphBuilder::buildWebs(rangesMap);
      }
      std::cout << "\n--- Generated Webs ---\n";
      std::cout << "Total Webs: " << webs.size() << "\n\n";
      for (const auto &web : webs) {
        std::cout << "  Web " << web.id << " [var: " << web.originalVariable
                  << "]  lines: ";
        for (size_t i = 0; i < web.activeLines.size(); ++i) {
          int line = web.activeLines[i];
          std::cout << line;
          // Mark def lines with +
          for (int d : web.defLines) {
            if (d == line) {
              std::cout << "+";
              break;
            }
          }
          // Mark use/end lines with -
          for (int u : web.useLines) {
            if (u == line) {
              std::cout << "-";
              break;
            }
          }
          if (i + 1 < web.activeLines.size())
            std::cout << ",";
        }
        std::cout << "\n";
      }
      break;
    }

    // ── 4. Build and Display Interference Graph ───────────────
    case 4: {
      if (!dataLoaded) {
        std::cerr << "Error: No data loaded. Use option 1 first.\n";
        break;
      }
      if (webs.empty()) {
        webs = GraphBuilder::buildWebs(rangesMap);
      }
      if (!graphBuilt) {
        interferenceGraph = GraphBuilder::buildInterferenceGraph(webs);
        graphBuilt = true;
      }

      std::cout << "\n--- Interference Graph ---\n";
      std::cout << "Vertices (Webs): " << interferenceGraph.getNumVertex()
                << "\n\n";

      // Display each web with its lines
      for (const auto &web : webs) {
        std::cout << "  Web " << web.id << " [var: " << web.originalVariable
                  << "]  lines: ";
        for (size_t i = 0; i < web.activeLines.size(); ++i) {
          int line = web.activeLines[i];
          std::cout << line;
          for (int d : web.defLines) {
            if (d == line) {
              std::cout << "+";
              break;
            }
          }
          for (int u : web.useLines) {
            if (u == line) {
              std::cout << "-";
              break;
            }
          }
          if (i + 1 < web.activeLines.size())
            std::cout << ",";
        }
        std::cout << "\n";
      }

      // Display adjacency list
      std::cout << "\nAdjacency list:\n";
      for (const auto &v : interferenceGraph.getVertexSet()) {
        int webId = v->getInfo();
        std::string varName = "?";
        for (const auto &w : webs) {
          if (w.id == webId) {
            varName = w.originalVariable;
            break;
          }
        }

        std::cout << "  Web " << webId << " (" << varName << ")";
        std::cout << " -> ";

        auto adj = v->getAdj();
        if (adj.empty()) {
          std::cout << "(no interferences)";
        } else {
          for (size_t i = 0; i < adj.size(); ++i) {
            int neighborId = adj[i]->getDest()->getInfo();
            std::string neighborVar = "?";
            for (const auto &w : webs) {
              if (w.id == neighborId) {
                neighborVar = w.originalVariable;
                break;
              }
            }
            std::cout << "Web " << neighborId << " (" << neighborVar << ")";
            if (i + 1 < adj.size())
              std::cout << ", ";
          }
        }
        std::cout << "\n";
      }
      break;
    }

      // ── 5. Run Basic Register Allocation ──────────────────────
      // ── 5. Run Basic Register Allocation ──────────────────────
    case 5: {
      if (!dataLoaded) {
        std::cerr << "Error: You must load data first (Option 1).\n";
        break;
      }

      // Garantir que webs e grafo estão construídos
      if (webs.empty()) {
        webs = GraphBuilder::buildWebs(rangesMap);
      }
      if (!graphBuilt) {
        interferenceGraph = GraphBuilder::buildInterferenceGraph(webs);
        graphBuilt = true;
      }

      if (outputFilePath.empty()) {
        std::cout << "Enter output file path: ";
        std::getline(std::cin, outputFilePath);
      }

      std::cout << "\nRunning Basic Register Allocation...\n";
      std::cout << "Available registers: " << registerConfig.numRegisters
                << "\n";

      // Correr o algoritmo (sem spills externos)
      ColoringResult result = GraphColoring::run(interferenceGraph, webs,
                                                 registerConfig.numRegisters,
                                                 {} // sem spills forçados
      );

      if (OutputWriter::writeResult(outputFilePath, webs, result, registerConfig.numRegisters)) {
        std::cout << "Results successfully written to " << outputFilePath << "\n";
      } else {
        std::cerr << "Error: Could not write results to " << outputFilePath << "\n";
      }

      if (!result.success) {
        std::cerr << "WARNING: Register allocation with "
                  << registerConfig.numRegisters
                  << " register(s) was not possible.\n";
      } else {
        std::cout << "Allocation successful using " << result.numColorsUsed
                  << " register(s).\n";
      }
      break;
    }
    // ── 6. Run Allocation with Web Spilling ───────────────────
    case 6: {
      if (!dataLoaded) {
        std::cerr << "Error: You must load data first (Option 1).\n";
        break;
      }
      if (outputFilePath.empty()) {
        std::cout << "Enter output file path: ";
        std::getline(std::cin, outputFilePath);
      }

      std::cout << "Running Allocation with Web Spilling...\n";
      std::cout << "Available registers: " << registerConfig.numRegisters << "\n";

      // Re-garantir que webs e grafo estão construídos
      if (webs.empty()) { webs = GraphBuilder::buildWebs(rangesMap); }
      if (!graphBuilt) { interferenceGraph = GraphBuilder::buildInterferenceGraph(webs); graphBuilt = true; }

      ColoringResult spillResult = SpillingAlgorithm::run(
          interferenceGraph, webs, registerConfig.numRegisters,
          registerConfig.algorithmParam);

      if (OutputWriter::writeResult(outputFilePath, webs, spillResult, registerConfig.numRegisters)) {
        std::cout << "Results with Spilling successfully written to " << outputFilePath << "\n";
      } else {
        std::cerr << "Error: Could not write results to " << outputFilePath << "\n";
      }

      if (!spillResult.success) {
        std::cout << "Note: Some webs were spilled to memory (labeled 'M' in output).\n";
      } else {
        std::cout << "Allocation successful using " << spillResult.numColorsUsed << " register(s).\n";
      }
      break;
    }

    // ── 7. Run Allocation with Web Splitting ──────────────────
    case 7:
      if (!dataLoaded) {
        std::cerr << "Error: You must load data first (Option 1).\n";
        break;
      }
      std::cout << "Running Allocation with Web Splitting...\n";
      // TODO: Implement splitting algorithm
      break;

    // ── 8. Run Free Algorithm ─────────────────────────────────
    case 8:
      if (!dataLoaded) {
        std::cerr << "Error: You must load data first (Option 1).\n";
        break;
      }
      std::cout << "Running Free Algorithm...\n";
      // TODO: Implement free algorithm
      break;

    // ── 0. Exit ───────────────────────────────────────────────
    case 0:
      std::cout << "Exiting...\n";
      break;

    default:
      std::cerr << "Invalid choice. Please select a valid option (0-8).\n";
      break;
    }
  }
}
