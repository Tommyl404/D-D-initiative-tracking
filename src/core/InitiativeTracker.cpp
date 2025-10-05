#include "core/InitiativeTracker.h"

#include <algorithm>
#include <cctype>

namespace initiative {

namespace {
std::string toLower(const std::string& value) {
    std::string copy = value;
    std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return copy;
}
}

void InitiativeTracker::setCombatants(std::vector<Combatant> combatants) {
    combatants_ = std::move(combatants);
    sortCombatants();
    currentIndex_.reset();
    ensureIndex();
}

void InitiativeTracker::addCombatant(const Combatant& combatant) {
    combatants_.push_back(combatant);
    sortCombatants();
}

std::vector<Combatant>& InitiativeTracker::combatants() {
    return combatants_;
}

const std::vector<Combatant>& InitiativeTracker::combatants() const {
    return combatants_;
}

void InitiativeTracker::sortCombatants() {
    std::sort(combatants_.begin(), combatants_.end(), compareCombatants);
    if (!combatants_.empty()) {
        ensureIndex();
    } else {
        currentIndex_.reset();
    }
}

std::optional<std::size_t> InitiativeTracker::currentIndex() const {
    return currentIndex_;
}

Combatant* InitiativeTracker::currentCombatant() {
    if (!currentIndex_) {
        return nullptr;
    }
    return &combatants_.at(*currentIndex_);
}

const Combatant* InitiativeTracker::currentCombatant() const {
    if (!currentIndex_) {
        return nullptr;
    }
    return &combatants_.at(*currentIndex_);
}

int InitiativeTracker::currentRound() const {
    return round_;
}

void InitiativeTracker::setRound(int round) {
    round_ = std::max(1, round);
}

void InitiativeTracker::nextTurn(bool skipUnconscious) {
    if (combatants_.empty()) {
        return;
    }

    ensureIndex();
    if (!currentIndex_) {
        return;
    }

    decrementConditions(combatants_.at(*currentIndex_));
    stepForward(skipUnconscious);
}

void InitiativeTracker::previousTurn(bool skipUnconscious) {
    if (combatants_.empty()) {
        return;
    }

    ensureIndex();
    if (!currentIndex_) {
        return;
    }

    stepBackward(skipUnconscious);
}

bool InitiativeTracker::compareCombatants(const Combatant& lhs, const Combatant& rhs) {
    if (lhs.initiative != rhs.initiative) {
        return lhs.initiative > rhs.initiative;
    }
    if (lhs.dexMod != rhs.dexMod) {
        return lhs.dexMod > rhs.dexMod;
    }
    if (lhs.isPC != rhs.isPC) {
        return lhs.isPC && !rhs.isPC;
    }
    return toLower(lhs.name) < toLower(rhs.name);
}

void InitiativeTracker::ensureIndex() {
    if (combatants_.empty()) {
        currentIndex_.reset();
        return;
    }
    if (!currentIndex_) {
        currentIndex_ = 0;
        if (combatants_.at(*currentIndex_).conscious) {
            return;
        }
        stepForward(true);
    }
}

void InitiativeTracker::stepForward(bool skipUnconscious) {
    if (combatants_.empty()) {
        currentIndex_.reset();
        return;
    }

    const std::size_t originalIndex = currentIndex_.value_or(0);
    std::size_t index = originalIndex;

    do {
        index = (index + 1) % combatants_.size();
        if (index == 0 && index != originalIndex) {
            ++round_;
        }
        if (!skipUnconscious || combatants_.at(index).conscious) {
            currentIndex_ = index;
            return;
        }
    } while (index != originalIndex);

    currentIndex_ = index;
}

void InitiativeTracker::stepBackward(bool skipUnconscious) {
    if (combatants_.empty()) {
        currentIndex_.reset();
        return;
    }

    const std::size_t originalIndex = currentIndex_.value_or(0);
    std::size_t index = originalIndex;

    do {
        index = (index + combatants_.size() - 1) % combatants_.size();
        if (index == combatants_.size() - 1 && index != originalIndex) {
            round_ = std::max(1, round_ - 1);
        }
        if (!skipUnconscious || combatants_.at(index).conscious) {
            currentIndex_ = index;
            return;
        }
    } while (index != originalIndex);

    currentIndex_ = index;
}

}
