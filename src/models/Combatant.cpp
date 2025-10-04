#include "Combatant.h"

void DeathSaves::reset() noexcept {
    successes = 0;
    failures = 0;
    dead = false;
    stable = false;
}

void DeathSaves::recordSuccess() noexcept {
    if (dead) {
        return;
    }
    if (successes < 3) {
        ++successes;
    }
    if (successes >= 3) {
        stable = true;
        failures = 0;
    }
}

void DeathSaves::recordFailure() noexcept {
    if (stable) {
        return;
    }
    if (failures < 3) {
        ++failures;
    }
    if (failures >= 3) {
        dead = true;
    }
}

bool operator==(const Condition &lhs, const Condition &rhs) noexcept {
    return lhs.name == rhs.name && lhs.remainingRounds == rhs.remainingRounds;
}

bool operator==(const DeathSaves &lhs, const DeathSaves &rhs) noexcept {
    return lhs.successes == rhs.successes && lhs.failures == rhs.failures && lhs.dead == rhs.dead && lhs.stable == rhs.stable;
}

bool operator==(const Combatant &lhs, const Combatant &rhs) noexcept {
    return lhs.id == rhs.id && lhs.name == rhs.name && lhs.initiative == rhs.initiative && lhs.dexMod == rhs.dexMod && lhs.isPC == rhs.isPC && lhs.conscious == rhs.conscious && lhs.hp == rhs.hp && lhs.ac == rhs.ac && lhs.deathSaves == rhs.deathSaves && lhs.conditions == rhs.conditions && lhs.notes == rhs.notes;
}

