#pragma once

#include <QString>
#include <QVector>

struct Condition {
    QString name;
    int remainingRounds = 0;

    bool isExpired() const noexcept { return remainingRounds <= 0; }
};

struct DeathSaves {
    int successes = 0;
    int failures = 0;
    bool dead = false;
    bool stable = false;

    void reset() noexcept;
    void recordSuccess() noexcept;
    void recordFailure() noexcept;
};

struct Combatant {
    int id = 0;
    QString name;
    int initiative = 0;
    int dexMod = 0;
    bool isPC = false;
    bool conscious = true;
    int hp = 0;
    int ac = 10;
    DeathSaves deathSaves;
    QVector<Condition> conditions;
    QString notes;
};

bool operator==(const Condition &lhs, const Condition &rhs) noexcept;
bool operator==(const DeathSaves &lhs, const DeathSaves &rhs) noexcept;
bool operator==(const Combatant &lhs, const Combatant &rhs) noexcept;

