#ifndef MENU_H
#define MENU_H

#include "../utils/DataLoader.h"
#include <string>
#include <unordered_map>

class Menu {
private:
    bool dataLoaded = false;
    RegisterConfig registerConfig;
    std::unordered_map<std::string, VariableRanges> rangesMap;

public:
    Menu();
    void run();
};

#endif // MENU_H
