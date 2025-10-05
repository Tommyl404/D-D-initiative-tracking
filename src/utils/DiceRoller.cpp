#include "utils/DiceRoller.h"

#include <algorithm>
#include <chrono>

namespace initiative {

DiceRoller::DiceRoller()
    : DiceRoller(static_cast<std::uint32_t>(
          std::chrono::high_resolution_clock::now().time_since_epoch().count())) {}

DiceRoller::DiceRoller(std::uint32_t seed)
    : engine_(seed) {}

int DiceRoller::rollD20(Mode mode) {
    const int first = distribution_(engine_);
    if (mode == Mode::Normal) {
        return first;
    }

    const int second = distribution_(engine_);
    if (mode == Mode::Advantage) {
        return std::max(first, second);
    }
    return std::min(first, second);
}

int DiceRoller::rollInitiative(int dexMod, Mode mode) {
    return rollD20(mode) + dexMod;
}

}
