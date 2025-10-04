#pragma once

#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>

#include "models/Combatant.h"

struct RosterCharacter {
    QString name;
    int dexMod = 0;
    bool isPC = false;
    QSet<QString> tags;
    int defaultHP = 0;
    int defaultAC = 10;
    QString defaultNotes;
};

struct RosterGroupEntry {
    QString characterName;
    int count = 1;
};

struct RosterGroup {
    QString name;
    QVector<RosterGroupEntry> entries;
};

struct MassAddNaming {
    QString pattern = "%name #%index";
    int startIndex = 1;
    bool zeroPad = false;
    int width = 2;
};

class RosterStore : public QObject {
    Q_OBJECT
public:
    explicit RosterStore(QObject *parent = nullptr);

    const QVector<RosterCharacter> &characters() const noexcept { return m_characters; }
    const QVector<RosterGroup> &groups() const noexcept { return m_groups; }
    const MassAddNaming &defaultNaming() const noexcept { return m_defaultNaming; }

    void setCharacters(QVector<RosterCharacter> characters);
    void setGroups(QVector<RosterGroup> groups);
    void setDefaultNaming(const MassAddNaming &naming);

    void setBasePath(QString path);

    bool load();
    bool save() const;

    QVector<RosterCharacter> filterCharacters(const QString &text, const QSet<QString> &tags) const;
    QVector<Combatant> massAdd(const QString &characterName, int count, const MassAddNaming &naming) const;
    QVector<Combatant> massAddGroup(const QString &groupName, const MassAddNaming &naming) const;

signals:
    void dataChanged();

private:
    QString charactersPath() const;
    QString groupsPath() const;

    QVector<RosterCharacter> m_characters;
    QVector<RosterGroup> m_groups;
    MassAddNaming m_defaultNaming;
    QString m_basePath;
};

