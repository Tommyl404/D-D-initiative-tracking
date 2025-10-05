#pragma once

#include <string>
#include <vector>

namespace initiative {

struct Condition {
    std::string name;
    int remainingRounds{0};
};

struct DeathSaves {
    int successes{0};
    int failures{0};
    bool dead{false};
    bool stable{false};

    void addSuccess();
    void addFailure();
    void reset();
};

struct Combatant {
    int id{0};
    std::string name;
    int initiative{0};
    int dexMod{0};
    bool isPC{false};
    bool conscious{true};
    int hp{0};
    int ac{0};
    DeathSaves deathSaves;
    std::vector<Condition> conditions;
    std::string notes;
};

void decrementConditions(Combatant& combatant);

}
