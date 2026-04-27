#ifndef DATALOADER_H
#define DATALOADER_H

#include <string>

class DataLoader {
public:
    DataLoader();
    void loadRanges(const std::string& filename);
    void loadRegisters(const std::string& filename);
};

#endif // DATALOADER_H
