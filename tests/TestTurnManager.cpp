#include <QtTest/QtTest>

#include <QDir>

#include "models/TurnManager.h"
#include "stores/EncounterStore.h"
#include "stores/RosterStore.h"

class TestTurnManager : public QObject {
    Q_OBJECT
private slots:
    void sortingRule();
    void conditionDecrement();
    void deathSavesLogic();
    void massAddNaming();
    void encounterRoundTrip();
};

void TestTurnManager::sortingRule() {
    TurnManager manager;
    Combatant a{1, "Alice", 15, 2, true};
    Combatant b{2, "Bob", 15, 4, false};
    Combatant c{3, "Charlie", 18, 1, false};
    manager.addCombatant(a);
    manager.addCombatant(b);
    manager.addCombatant(c);
    const auto &list = manager.combatants();
    QCOMPARE(list[0].name, QStringLiteral("Charlie"));
    QCOMPARE(list[1].name, QStringLiteral("Alice"));
    QCOMPARE(list[2].name, QStringLiteral("Bob"));
}

void TestTurnManager::conditionDecrement() {
    TurnManager manager;
    Combatant a;
    a.id = 1;
    a.name = "Test";
    a.conditions.append({"Poison", 1});
    manager.addCombatant(a);
    QVERIFY(manager.advanceTurn());
    QCOMPARE(manager.combatants().first().conditions.size(), 0);
}

void TestTurnManager::deathSavesLogic() {
    DeathSaves saves;
    saves.recordFailure();
    saves.recordFailure();
    QCOMPARE(saves.failures, 2);
    QVERIFY(!saves.dead);
    saves.recordFailure();
    QVERIFY(saves.dead);
    saves.reset();
    saves.recordSuccess();
    saves.recordSuccess();
    saves.recordSuccess();
    QVERIFY(saves.stable);
    QCOMPARE(saves.failures, 0);
}

void TestTurnManager::massAddNaming() {
    RosterStore store;
    MassAddNaming naming;
    naming.pattern = "%name %index";
    naming.startIndex = 1;
    naming.zeroPad = true;
    naming.width = 2;

    RosterCharacter character;
    character.name = "Bandit";
    character.dexMod = 2;
    character.defaultHP = 11;
    character.defaultAC = 12;
    character.defaultNotes = "Sneaky";
    store.setCharacters({character});
    store.setDefaultNaming(naming);

    const auto added = store.massAdd("Bandit", 3, naming);
    QCOMPARE(added.size(), 3);
    QCOMPARE(added[0].name, QStringLiteral("Bandit 01"));
    QCOMPARE(added[1].name, QStringLiteral("Bandit 02"));
    QCOMPARE(added[2].hp, 11);
    QCOMPARE(added[2].notes, QStringLiteral("Sneaky"));
}

void TestTurnManager::encounterRoundTrip() {
    TurnManager manager;
    Combatant a{1, "Alice", 15, 2, true};
    a.hp = 25;
    a.conditions.append({"Bless", 3});
    manager.addCombatant(a);
    Combatant b{2, "Bob", 12, 1, false};
    b.conscious = false;
    b.deathSaves.recordFailure();
    manager.addCombatant(b);

    int round = 2;
    int turnIndex = 1;
    const auto data = EncounterStore::serialize(manager, round, turnIndex);

    TurnManager restored;
    int restoredRound = 0;
    int restoredTurnIndex = 0;
    QVERIFY(EncounterStore::deserialize(data, restored, restoredRound, restoredTurnIndex));
    QCOMPARE(restoredRound, round);
    QCOMPARE(restoredTurnIndex, turnIndex);
    QCOMPARE(restored.combatants().size(), manager.combatants().size());
    QCOMPARE(restored.combatants()[0].conditions.size(), 1);
    QCOMPARE(restored.combatants()[1].deathSaves.failures, 1);
}

QTEST_MAIN(TestTurnManager)
#include "TestTurnManager.moc"

