#ifndef LIVERANGE_H
#define LIVERANGE_H

#include <string>
#include <vector>

struct LiveRangePoint {
    int line;
    bool isDef; //se é a linha onde foi definido
    bool isEnd; //se é a linha onde morreu
};

struct VariableRanges {
    std::string varName;
    std::vector<std::vector<LiveRangePoint>> ranges;
};

#endif // LIVERANGE_H
