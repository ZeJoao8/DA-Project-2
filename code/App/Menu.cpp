#include "Menu.h"
#include "../utils/DataLoader.h"
#include <iostream>
#include <limits>

Menu::Menu() {
    // Constructor
}

void Menu::run() {
    int choice = -1;
    DataLoader dataLoader;
    std::string input;

    while (choice != 0) {
        std::cout << "\n--- Compiler Register Allocation Menu ---\n";
        std::cout << "1. Load Data (ranges and registers files)\n";
        std::cout << "2. Run Basic Register Allocation\n";
        std::cout << "3. Run Allocation with Web Spilling\n";
        std::cout << "4. Run Allocation with Web Splitting\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";

        std::getline(std::cin, input);

        try {
            choice = std::stoi(input);
        } catch (...) {
            std::cerr << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1:
                std::cout << "Loading data...\n";
                dataLoader.loadRanges("ranges.txt");
                dataLoader.loadRegisters("registers.txt");
                break;
            case 2:
                std::cout << "Running Basic Register Allocation...\n";
                break;
            case 3:
                std::cout << "Running Allocation with Web Spilling...\n";
                break;
            case 4:
                std::cout << "Running Allocation with Web Splitting...\n";
                break;
            case 0:
                std::cout << "Exiting...\n";
                break;
            default:
                std::cerr << "Invalid choice. Please select a valid option (0-4).\n";
                break;
        }
    }
}
