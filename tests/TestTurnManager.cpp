#include "core/InitiativeTracker.h"
#include "storage/EncounterStore.h"
#include "storage/RosterStore.h"

#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace initiative;

namespace {
struct TestFailure : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

#define EXPECT_TRUE(expr)                                                                      \
    do {                                                                                       \
        if (!(expr)) {                                                                         \
            throw TestFailure(std::string("Expectation failed: ") + #expr + " at " + __FILE__ + ":" + std::to_string(__LINE__)); \
        }                                                                                      \
    } while (false)

#define EXPECT_EQ(lhs, rhs) EXPECT_TRUE((lhs) == (rhs))

void testInitiativeSorting() {
    Combatant a{1, "Bandit", 12, 2, false, true, 11, 12};
    Combatant b{2, "alpha", 15, 2, false, true, 11, 12};
    Combatant c{3, "Hero", 15, 2, true, true, 24, 16};
    Combatant d{4, "Scout", 15, 3, false, true, 10, 13};
    Combatant e{5, "Bravo", 18, 0, false, true, 8, 13};

    InitiativeTracker tracker;
    tracker.setCombatants({a, b, c, d, e});

    const auto& sorted = tracker.combatants();
    EXPECT_EQ(sorted.size(), 5U);
    EXPECT_EQ(sorted[0].name, "Bravo");
    EXPECT_EQ(sorted[1].name, "Scout");
    EXPECT_EQ(sorted[2].name, "Hero");
    EXPECT_EQ(sorted[3].name, "alpha");
    EXPECT_EQ(sorted[4].name, "Bandit");
}

void testConditionDecrement() {
    Combatant acting{1, "Caster", 15, 3, true, true, 30, 15};
    acting.conditions = {{"Bless", 1}, {"Shield of Faith", 2}};
    Combatant ally{2, "Ally", 14, 1, true, true, 20, 15};

    InitiativeTracker tracker;
    tracker.setCombatants({ally, acting});
    tracker.setRound(1);

    tracker.nextTurn();

    const auto& combatants = tracker.combatants();
    const auto& updated = combatants.front();
    EXPECT_EQ(updated.conditions.size(), 1U);
    EXPECT_EQ(updated.conditions.front().name, "Shield of Faith");
    EXPECT_EQ(updated.conditions.front().remainingRounds, 1);
}

void testDeathSaves() {
    DeathSaves saves;
    saves.addSuccess();
    saves.addSuccess();
    saves.addSuccess();
    EXPECT_TRUE(saves.stable);
    EXPECT_TRUE(!saves.dead);

    saves.reset();
    saves.addFailure();
    saves.addFailure();
    saves.addFailure();
    EXPECT_TRUE(saves.dead);
}

void testMassAddNaming() {
    const auto dataDir = std::filesystem::path(__FILE__).parent_path() / ".." / "data";
    RosterStore roster;
    roster.loadCharacters(dataDir / "characters.json");
    roster.loadGroups(dataDir / "groups.json");

    int nextId = 10;
    auto combatants = roster.instantiateGroup("Bandit Ambush", nextId);

    EXPECT_EQ(combatants.size(), 4U);
    EXPECT_EQ(combatants[0].name, "Bandit 01");
    EXPECT_EQ(combatants[1].name, "Bandit 02");
    EXPECT_EQ(combatants[2].name, "Bandit 03");
    EXPECT_EQ(combatants[3].name, "Bandit 04");
}

void testEncounterRoundTrip() {
    EncounterData encounter;
    encounter.round = 2;
    encounter.turnIndex = 1;
    encounter.combatants = {
        Combatant{42, "Vaelen", 17, 3, true, true, 27, 17},
        Combatant{43, "Bandit", 13, 2, false, true, 11, 12},
    };
    encounter.combatants[0].conditions = {{"Bless", 3}};
    encounter.combatants[1].deathSaves.failures = 1;

    const auto tempPath = std::filesystem::temp_directory_path() / "encounter_roundtrip.json";
    EncounterStore::saveToFile(encounter, tempPath);
    const auto loaded = EncounterStore::loadFromFile(tempPath);
    std::filesystem::remove(tempPath);

    EXPECT_EQ(loaded.schema, encounter.schema);
    EXPECT_EQ(loaded.round, encounter.round);
    EXPECT_EQ(loaded.turnIndex, encounter.turnIndex);
    EXPECT_EQ(loaded.combatants.size(), encounter.combatants.size());
    for (std::size_t i = 0; i < loaded.combatants.size(); ++i) {
        const auto& lhs = loaded.combatants[i];
        const auto& rhs = encounter.combatants[i];
        EXPECT_EQ(lhs.id, rhs.id);
        EXPECT_EQ(lhs.name, rhs.name);
        EXPECT_EQ(lhs.initiative, rhs.initiative);
        EXPECT_EQ(lhs.dexMod, rhs.dexMod);
        EXPECT_EQ(lhs.isPC, rhs.isPC);
        EXPECT_EQ(lhs.conscious, rhs.conscious);
        EXPECT_EQ(lhs.hp, rhs.hp);
        EXPECT_EQ(lhs.ac, rhs.ac);
        EXPECT_EQ(lhs.notes, rhs.notes);
        EXPECT_EQ(lhs.deathSaves.successes, rhs.deathSaves.successes);
        EXPECT_EQ(lhs.deathSaves.failures, rhs.deathSaves.failures);
        EXPECT_EQ(lhs.deathSaves.dead, rhs.deathSaves.dead);
        EXPECT_EQ(lhs.deathSaves.stable, rhs.deathSaves.stable);
        EXPECT_EQ(lhs.conditions.size(), rhs.conditions.size());
        for (std::size_t c = 0; c < lhs.conditions.size(); ++c) {
            EXPECT_EQ(lhs.conditions[c].name, rhs.conditions[c].name);
            EXPECT_EQ(lhs.conditions[c].remainingRounds, rhs.conditions[c].remainingRounds);
        }
    }
}

#undef EXPECT_EQ
#undef EXPECT_TRUE

} // namespace

int main() {
    struct TestCase {
        std::string name;
        std::function<void()> func;
    };

    const std::vector<TestCase> tests = {
        {"Initiative sorting", testInitiativeSorting},
        {"Condition decrement", testConditionDecrement},
        {"Death saves", testDeathSaves},
        {"Mass add naming", testMassAddNaming},
        {"Encounter round trip", testEncounterRoundTrip},
    };

    int failures = 0;
    for (const auto& test : tests) {
        try {
            test.func();
            std::cout << "[PASS] " << test.name << "\n";
        } catch (const std::exception& ex) {
            ++failures;
            std::cout << "[FAIL] " << test.name << ": " << ex.what() << "\n";
        }
    }

    std::cout << tests.size() - failures << "/" << tests.size() << " tests passed." << std::endl;
    return failures == 0 ? 0 : 1;
}
