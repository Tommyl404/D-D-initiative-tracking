#include "core/Combatant.h"

#include <algorithm>

namespace initiative {

namespace {
constexpr int kMaxDeathSaveCount = 3;
}

void DeathSaves::addSuccess() {
    if (dead) {
        return;
    }
    successes = std::min(successes + 1, kMaxDeathSaveCount);
    if (successes >= kMaxDeathSaveCount) {
        stable = true;
    }
}

void DeathSaves::addFailure() {
    if (stable) {
        return;
    }
    failures = std::min(failures + 1, kMaxDeathSaveCount);
    if (failures >= kMaxDeathSaveCount) {
        dead = true;
    }
}

void DeathSaves::reset() {
    successes = 0;
    failures = 0;
    dead = false;
    stable = false;
}

void decrementConditions(Combatant& combatant) {
    for (auto& condition : combatant.conditions) {
        if (condition.remainingRounds > 0) {
            --condition.remainingRounds;
        }
    }

    auto remover = [](const Condition& condition) {
        return condition.remainingRounds <= 0;
    };
    combatant.conditions.erase(
        std::remove_if(combatant.conditions.begin(), combatant.conditions.end(), remover),
        combatant.conditions.end());
}

}
