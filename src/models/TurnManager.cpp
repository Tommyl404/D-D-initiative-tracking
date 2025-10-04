#include "TurnManager.h"

#include <algorithm>

TurnManager::CombatantList &TurnManager::combatants() {
    return m_combatants;
}

const TurnManager::CombatantList &TurnManager::combatants() const {
    return m_combatants;
}

void TurnManager::setCombatants(CombatantList list) {
    m_combatants = std::move(list);
    sortCombatants();
    normalizeTurnIndex();
}

void TurnManager::addCombatant(const Combatant &combatant) {
    m_combatants.push_back(combatant);
    sortCombatants();
    normalizeTurnIndex();
}

bool TurnManager::removeCombatant(int id) {
    const auto it = std::remove_if(m_combatants.begin(), m_combatants.end(), [id](const Combatant &c) { return c.id == id; });
    if (it == m_combatants.end()) {
        return false;
    }
    const int removedIndex = std::distance(m_combatants.begin(), it);
    m_combatants.erase(it, m_combatants.end());
    if (m_turnIndex >= static_cast<int>(m_combatants.size())) {
        m_turnIndex = std::max(0, static_cast<int>(m_combatants.size()) - 1);
    }
    if (removedIndex <= m_turnIndex) {
        normalizeTurnIndex();
    }
    return true;
}

std::optional<Combatant> TurnManager::combatantById(int id) const {
    const auto it = std::find_if(m_combatants.begin(), m_combatants.end(), [id](const Combatant &c) { return c.id == id; });
    if (it == m_combatants.end()) {
        return std::nullopt;
    }
    return *it;
}

static bool combatantLess(const Combatant &lhs, const Combatant &rhs) {
    if (lhs.initiative != rhs.initiative) {
        return lhs.initiative > rhs.initiative;
    }
    if (lhs.dexMod != rhs.dexMod) {
        return lhs.dexMod > rhs.dexMod;
    }
    if (lhs.isPC != rhs.isPC) {
        return lhs.isPC && !rhs.isPC;
    }
    const auto lhsName = lhs.name.toCaseFolded();
    const auto rhsName = rhs.name.toCaseFolded();
    return lhsName < rhsName;
}

void TurnManager::sortCombatants() {
    std::stable_sort(m_combatants.begin(), m_combatants.end(), combatantLess);
}

bool TurnManager::advanceTurn() {
    if (m_combatants.isEmpty()) {
        return false;
    }

    decrementConditionsForCurrent();

    const int size = m_combatants.size();
    int attempts = 0;
    do {
        ++m_turnIndex;
        if (m_turnIndex >= size) {
            m_turnIndex = 0;
            ++m_round;
        }
        ++attempts;
        if (!m_skipUnconscious) {
            break;
        }
    } while (attempts <= size && !m_combatants[m_turnIndex].conscious);

    return true;
}

bool TurnManager::rewindTurn() {
    if (m_combatants.isEmpty()) {
        return false;
    }

    const int size = m_combatants.size();
    int attempts = 0;
    do {
        --m_turnIndex;
        if (m_turnIndex < 0) {
            m_turnIndex = size - 1;
            m_round = std::max(1, m_round - 1);
        }
        ++attempts;
        if (!m_skipUnconscious) {
            break;
        }
    } while (attempts <= size && !m_combatants[m_turnIndex].conscious);

    return true;
}

void TurnManager::forEachCombatant(const std::function<void(Combatant &)> &visitor) {
    for (auto &combatant : m_combatants) {
        visitor(combatant);
    }
}

void TurnManager::resetInitiativeOrder() {
    m_round = 1;
    m_turnIndex = 0;
    sortCombatants();
}

void TurnManager::decrementConditionsForCurrent() {
    if (m_combatants.isEmpty()) {
        return;
    }
    Combatant &current = m_combatants[m_turnIndex];
    for (auto &condition : current.conditions) {
        if (condition.remainingRounds > 0) {
            --condition.remainingRounds;
        }
    }
    current.conditions.erase(std::remove_if(current.conditions.begin(), current.conditions.end(), [](const Condition &c) {
        return c.isExpired();
    }), current.conditions.end());
}

void TurnManager::normalizeTurnIndex() {
    if (m_combatants.isEmpty()) {
        m_turnIndex = 0;
        m_round = 1;
        return;
    }
    if (m_turnIndex >= m_combatants.size()) {
        m_turnIndex = m_combatants.size() - 1;
    }
    if (m_turnIndex < 0) {
        m_turnIndex = 0;
    }
    if (m_round < 1) {
        m_round = 1;
    }
}

