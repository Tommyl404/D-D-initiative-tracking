#pragma once

#include <QObject>
#include <QRandomGenerator>

enum class RollMode {
    Normal,
    Advantage,
    Disadvantage
};

class DiceRoller : public QObject {
    Q_OBJECT
public:
    explicit DiceRoller(QObject *parent = nullptr);

    void setSeed(quint32 seed);
    int rollD20(RollMode mode, int modifier = 0);

signals:
    void rollPerformed(int raw, RollMode mode, int modifier, int total);

private:
    QRandomGenerator m_rng;
};

