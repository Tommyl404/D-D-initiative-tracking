#pragma once

#include "core/Combatant.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace initiative {

class InitiativeTracker {
public:
    void setCombatants(std::vector<Combatant> combatants);
    void addCombatant(const Combatant& combatant);

    std::vector<Combatant>& combatants();
    const std::vector<Combatant>& combatants() const;

    void sortCombatants();

    std::optional<std::size_t> currentIndex() const;
    Combatant* currentCombatant();
    const Combatant* currentCombatant() const;

    int currentRound() const;
    void setRound(int round);

    void nextTurn(bool skipUnconscious = true);
    void previousTurn(bool skipUnconscious = true);

private:
    static bool compareCombatants(const Combatant& lhs, const Combatant& rhs);
    void ensureIndex();
    void stepForward(bool skipUnconscious);
    void stepBackward(bool skipUnconscious);

    std::vector<Combatant> combatants_;
    std::optional<std::size_t> currentIndex_;
    int round_{1};
};

}
