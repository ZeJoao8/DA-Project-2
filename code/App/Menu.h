#ifndef MENU_H
#define MENU_H

#include "../utils/DataLoader.h"
#include "../utils/GraphBuilder.h"
#include <string>
#include <unordered_map>
#include <vector>

class Menu {
private:
    bool dataLoaded = false;
    RegisterConfig registerConfig;
    std::unordered_map<std::string, VariableRanges> rangesMap;
    std::vector<Web> webs;
    Graph<int> interferenceGraph;
    bool graphBuilt = false;

public:
    Menu();
    void run();
};

#endif // MENU_H
