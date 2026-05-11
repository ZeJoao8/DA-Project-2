#ifndef WEB_H
#define WEB_H

#include <string>
#include <vector>

struct Web {
    int id;
    std::string originalVariable;
    std::vector<int> activeLines;
    std::vector<int> defLines;
    std::vector<int> useLines;
};

#endif // WEB_H
