#pragma once

#include <random>

namespace initiative {

class DiceRoller {
public:
    enum class Mode { Normal, Advantage, Disadvantage };

    DiceRoller();
    explicit DiceRoller(std::uint32_t seed);

    int rollD20(Mode mode = Mode::Normal);
    int rollInitiative(int dexMod, Mode mode = Mode::Normal);

private:
    std::mt19937 engine_;
    std::uniform_int_distribution<int> distribution_{1, 20};
};

}
