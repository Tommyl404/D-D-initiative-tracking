#pragma once

#include "Combatant.h"

#include <QVector>
#include <functional>
#include <optional>

class TurnManager {
public:
    using CombatantList = QVector<Combatant>;

    CombatantList &combatants();
    const CombatantList &combatants() const;

    void setCombatants(CombatantList list);

    void addCombatant(const Combatant &combatant);
    bool removeCombatant(int id);
    std::optional<Combatant> combatantById(int id) const;

    void sortCombatants();

    int round() const noexcept { return m_round; }
    int turnIndex() const noexcept { return m_turnIndex; }

    bool advanceTurn();
    bool rewindTurn();

    void forEachCombatant(const std::function<void(Combatant &)> &visitor);

    void resetInitiativeOrder();

    void decrementConditionsForCurrent();

    void setSkipUnconscious(bool skip) noexcept { m_skipUnconscious = skip; }
    bool skipUnconscious() const noexcept { return m_skipUnconscious; }

private:
    void normalizeTurnIndex();

    CombatantList m_combatants;
    int m_round = 1;
    int m_turnIndex = 0;
    bool m_skipUnconscious = true;
};

