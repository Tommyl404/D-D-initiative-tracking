#include "DiceRoller.h"

DiceRoller::DiceRoller(QObject *parent)
    : QObject(parent)
    , m_rng(QRandomGenerator::securelySeeded()) {}

void DiceRoller::setSeed(quint32 seed) {
    m_rng = QRandomGenerator(seed);
}

int DiceRoller::rollD20(RollMode mode, int modifier) {
    auto rollOnce = [this]() { return static_cast<int>(m_rng.bounded(1, 21)); };
    int first = rollOnce();
    int result = first;
    if (mode == RollMode::Advantage || mode == RollMode::Disadvantage) {
        int second = rollOnce();
        if (mode == RollMode::Advantage) {
            result = std::max(first, second);
        } else {
            result = std::min(first, second);
        }
    }
    int total = result + modifier;
    emit rollPerformed(result, mode, modifier, total);
    return total;
}

