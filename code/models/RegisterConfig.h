/**
 * @file RegisterConfig.h
 * @brief Defines the RegisterConfig struct for algorithm configuration.
 *
 * Populated by DataLoader::loadRegisters() from the `registers.txt` input
 * file. Consumed by AllocationEngine to select and parameterise the correct
 * allocation algorithm.
 *
 */

#ifndef REGISTERCONFIG_H
#define REGISTERCONFIG_H

#include <string>

/**
 * @brief Configuration parsed from the registers input file.
 *
 * Controls which algorithm is used and with how many physical registers.
 */
struct RegisterConfig {
    int numRegisters = 0;       ///< Number of available physical registers (K colors).
    std::string algorithmName;  ///< Algorithm identifier: "basic", "spilling", "splitting", or "free".
    int algorithmParam = 0;     ///< Optional algorithm-specific parameter (e.g., max spills / max splits).
};

#endif // REGISTERCONFIG_H
