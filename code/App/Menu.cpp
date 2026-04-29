#include "Menu.h"
#include "../utils/DataLoader.h"
#include <iostream>
#include <limits>
#include <string>

Menu::Menu() {
  // Constructor
}

void Menu::run() {
  int choice = -1;

  while (choice != 0) {
    std::cout << "\n--- Compiler Register Allocation Menu ---\n";
    std::cout << "1. Load Data (ranges and registers files)\n";
    std::cout << "2. Run Basic Register Allocation\n";
    std::cout << "3. Run Allocation with Web Spilling\n";
    std::cout << "4. Run Allocation with Web Splitting\n";
    std::cout << "5. Display Loaded Data (Ranges and Registers)\n";
    std::cout << "0. Exit\n";
    std::cout << "Enter your choice: ";

    if (!(std::cin >> choice)) {
      std::cerr << "Invalid input. Please enter a number.\n";
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }

    // Clear the newline character left in the buffer after reading the integer
    // so that subsequent std::getline calls don't skip automatically.
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (choice) {
    case 1: {
      std::string rangesPath, registersPath;

      std::cout << "Enter ranges file path: ";
      std::getline(std::cin, rangesPath);

      std::cout << "Enter registers file path: ";
      std::getline(std::cin, registersPath);

      std::cout << "Loading data...\n";
      // Reset state before loading new data
      dataLoaded = false;
      rangesMap.clear();
      registerConfig = RegisterConfig();

      try {
        rangesPath = DataLoader::resolvePath(rangesPath, "ranges");
        registersPath = DataLoader::resolvePath(registersPath, "registers");

        rangesMap = DataLoader::loadRanges(rangesPath);
        registerConfig = DataLoader::loadRegisters(registersPath);
        dataLoaded = true;
        std::cout << "Successfully loaded " << rangesMap.size()
                  << " variables and " << registerConfig.numRegisters
                  << " registers.\n";
      } catch (const std::exception &e) {
        std::cerr << "Error: Failed to load data. Returning to menu.\n";
      }
      break;
    }
    case 2:
      if (!dataLoaded) {
        std::cerr << "Error: You must load data first (Option 1) before "
                     "running algorithms.\n";
        break;
      }
      std::cout << "Running Basic Register Allocation...\n";
      break;
    case 3:
      if (!dataLoaded) {
        std::cerr << "Error: You must load data first (Option 1) before "
                     "running algorithms.\n";
        break;
      }
      std::cout << "Running Allocation with Web Spilling...\n";
      break;
    case 4:
      if (!dataLoaded) {
        std::cerr << "Error: You must load data first (Option 1) before "
                     "running algorithms.\n";
        break;
      }
      std::cout << "Running Allocation with Web Splitting...\n";
      break;
    case 5: {
      if (!dataLoaded) {
        std::cerr << "Error: No data loaded. Use option 1 first.\n";
        break;
      }
      std::cout << "\n--- Register Configuration ---\n";
      std::cout << "Registers: " << registerConfig.numRegisters << "\n";
      std::cout << "Algorithm: " << registerConfig.algorithmName << "\n";
      std::cout << "Param:     " << registerConfig.algorithmParam << "\n";

      std::cout << "\n--- Variable Ranges ---\n";
      for (const auto &[name, varRanges] : rangesMap) {
        std::cout << "Variable '" << name << "':\n";
        for (size_t i = 0; i < varRanges.ranges.size(); ++i) {
          std::cout << "  Segment " << i + 1 << ": ";
          for (const auto &point : varRanges.ranges[i]) {
            std::cout << "L" << point.line;
            if (point.isDef)
              std::cout << "(D)";
            if (point.isEnd)
              std::cout << "(E)"; // E for End
            std::cout << " ";
          }
          std::cout << "\n";
        }
      }
      break;
    }
    case 0:
      std::cout << "Exiting...\n";
      break;
    default:
      std::cerr << "Invalid choice. Please select a valid option (0-5).\n";
      break;
    }
  }
}
