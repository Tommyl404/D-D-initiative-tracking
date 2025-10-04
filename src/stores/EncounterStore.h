#pragma once

#include <QObject>
#include <QString>

#include "models/TurnManager.h"

class EncounterStore : public QObject {
    Q_OBJECT
public:
    explicit EncounterStore(QObject *parent = nullptr);

    void setFilePath(QString path);
    QString filePath() const { return m_filePath; }

    bool load(TurnManager &manager, int &round, int &turnIndex) const;
    bool save(const TurnManager &manager, int round, int turnIndex) const;

    static QByteArray serialize(const TurnManager &manager, int round, int turnIndex);
    static bool deserialize(const QByteArray &data, TurnManager &manager, int &round, int &turnIndex);

private:
    QString m_filePath;
};

