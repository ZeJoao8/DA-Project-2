#include <iostream>
#include <string>
#include "App/BatchMode.h"
#include "App/Menu.h"

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
