#include "core/InitiativeTracker.h"
#include "storage/EncounterStore.h"
#include "storage/RosterStore.h"
#include "utils/DiceRoller.h"

#include <filesystem>
#include <iostream>

int main() {
    using namespace initiative;

    std::cout << "D&D Initiative Tracker (console prototype)" << std::endl;

    try {
        const std::filesystem::path encounterPath{"data/sample_encounter.json"};
        EncounterData encounter = EncounterStore::loadFromFile(encounterPath);

        InitiativeTracker tracker;
        tracker.setCombatants(encounter.combatants);
        tracker.setRound(encounter.round);

        if (auto current = tracker.currentCombatant()) {
            std::cout << "Round " << tracker.currentRound() << ": " << current->name << " acts first." << std::endl;
        }

        tracker.nextTurn();
        if (auto current = tracker.currentCombatant()) {
            std::cout << "Next up: " << current->name << std::endl;
        }

        RosterStore roster;
        roster.loadCharacters("data/characters.json");
        roster.loadGroups("data/groups.json");

        int nextId = 100;
        auto bandits = roster.instantiateGroup("Bandit Ambush", nextId);
        std::cout << "Loaded roster group with " << bandits.size() << " combatants:" << std::endl;
        for (const auto& combatant : bandits) {
            std::cout << "  - " << combatant.name << " (DEX " << combatant.dexMod << ")" << std::endl;
        }

        DiceRoller roller{12345};
        const int roll = roller.rollInitiative(3, DiceRoller::Mode::Advantage);
        std::cout << "Sample initiative roll (advantage, DEX +3): " << roll << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
