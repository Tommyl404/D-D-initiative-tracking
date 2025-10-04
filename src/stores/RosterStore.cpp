#include "RosterStore.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#include <algorithm>

namespace {
constexpr int kSchemaVersion = 2;

QJsonObject toJson(const RosterCharacter &character) {
    QJsonObject obj;
    obj["name"] = character.name;
    obj["dexMod"] = character.dexMod;
    obj["isPC"] = character.isPC;
    QStringList tagsList;
    tagsList.reserve(character.tags.size());
    for (const auto &tag : character.tags) {
        tagsList.append(tag);
    }
    obj["tags"] = QJsonArray::fromStringList(tagsList);
    obj["defaultHP"] = character.defaultHP;
    obj["defaultAC"] = character.defaultAC;
    obj["defaultNotes"] = character.defaultNotes;
    return obj;
}

RosterCharacter characterFromJson(const QJsonObject &obj) {
    RosterCharacter character;
    character.name = obj.value("name").toString();
    character.dexMod = obj.value("dexMod").toInt();
    character.isPC = obj.value("isPC").toBool();
    for (const auto &tag : obj.value("tags").toArray()) {
        character.tags.insert(tag.toString());
    }
    character.defaultHP = obj.value("defaultHP").toInt();
    character.defaultAC = obj.value("defaultAC").toInt();
    character.defaultNotes = obj.value("defaultNotes").toString();
    return character;
}

QJsonObject toJson(const RosterGroup &group) {
    QJsonObject obj;
    obj["name"] = group.name;
    QJsonArray entries;
    for (const auto &entry : group.entries) {
        QJsonObject entryObj;
        entryObj["character"] = entry.characterName;
        entryObj["count"] = entry.count;
        entries.push_back(entryObj);
    }
    obj["entries"] = entries;
    return obj;
}

RosterGroup groupFromJson(const QJsonObject &obj) {
    RosterGroup group;
    group.name = obj.value("name").toString();
    for (const auto &entryValue : obj.value("entries").toArray()) {
        const auto entryObj = entryValue.toObject();
        RosterGroupEntry entry;
        entry.characterName = entryObj.value("character").toString();
        entry.count = entryObj.value("count").toInt(1);
        group.entries.push_back(entry);
    }
    return group;
}
}

RosterStore::RosterStore(QObject *parent)
    : QObject(parent) {
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataLocation.isEmpty()) {
        dataLocation = QDir::homePath() + "/.dnd-initiative";
    }
    setBasePath(dataLocation);
}

void RosterStore::setBasePath(QString path) {
    m_basePath = std::move(path);
    QDir().mkpath(m_basePath);
}

void RosterStore::setCharacters(QVector<RosterCharacter> characters) {
    m_characters = std::move(characters);
    emit dataChanged();
}

void RosterStore::setGroups(QVector<RosterGroup> groups) {
    m_groups = std::move(groups);
    emit dataChanged();
}

void RosterStore::setDefaultNaming(const MassAddNaming &naming) {
    m_defaultNaming = naming;
    emit dataChanged();
}

QString RosterStore::charactersPath() const {
    return m_basePath + "/characters.json";
}

QString RosterStore::groupsPath() const {
    return m_basePath + "/groups.json";
}

bool RosterStore::load() {
    QFile charactersFile(charactersPath());
    if (charactersFile.exists() && charactersFile.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(charactersFile.readAll());
        const auto root = doc.object();
        const auto schema = root.value("schema").toInt();
        if (schema == kSchemaVersion) {
            m_characters.clear();
            for (const auto &value : root.value("characters").toArray()) {
                m_characters.push_back(characterFromJson(value.toObject()));
            }
        }
    }

    QFile groupsFile(groupsPath());
    if (groupsFile.exists() && groupsFile.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(groupsFile.readAll());
        const auto root = doc.object();
        const auto schema = root.value("schema").toInt();
        if (schema == kSchemaVersion) {
            m_groups.clear();
            for (const auto &value : root.value("groups").toArray()) {
                m_groups.push_back(groupFromJson(value.toObject()));
            }
            const auto namingObj = root.value("naming").toObject();
            if (!namingObj.isEmpty()) {
                m_defaultNaming.pattern = namingObj.value("pattern").toString(m_defaultNaming.pattern);
                m_defaultNaming.startIndex = namingObj.value("startIndex").toInt(m_defaultNaming.startIndex);
                m_defaultNaming.zeroPad = namingObj.value("zeroPad").toBool(m_defaultNaming.zeroPad);
                m_defaultNaming.width = namingObj.value("width").toInt(m_defaultNaming.width);
            }
        }
    }

    emit dataChanged();
    return true;
}

bool RosterStore::save() const {
    QJsonObject charactersRoot;
    charactersRoot["schema"] = kSchemaVersion;
    QJsonArray characterArray;
    for (const auto &character : m_characters) {
        characterArray.push_back(toJson(character));
    }
    charactersRoot["characters"] = characterArray;
    QFile charactersFile(charactersPath());
    if (!charactersFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    charactersFile.write(QJsonDocument(charactersRoot).toJson(QJsonDocument::Indented));

    QJsonObject groupsRoot;
    groupsRoot["schema"] = kSchemaVersion;
    QJsonArray groupArray;
    for (const auto &group : m_groups) {
        groupArray.push_back(toJson(group));
    }
    groupsRoot["groups"] = groupArray;
    QJsonObject naming;
    naming["pattern"] = m_defaultNaming.pattern;
    naming["startIndex"] = m_defaultNaming.startIndex;
    naming["zeroPad"] = m_defaultNaming.zeroPad;
    naming["width"] = m_defaultNaming.width;
    groupsRoot["naming"] = naming;

    QFile groupsFile(groupsPath());
    if (!groupsFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    groupsFile.write(QJsonDocument(groupsRoot).toJson(QJsonDocument::Indented));

    return true;
}

QVector<RosterCharacter> RosterStore::filterCharacters(const QString &text, const QSet<QString> &tags) const {
    QVector<RosterCharacter> results;
    const auto lower = text.toCaseFolded();
    for (const auto &character : m_characters) {
        if (!text.isEmpty() && !character.name.toCaseFolded().contains(lower)) {
            continue;
        }
        if (!tags.isEmpty()) {
            bool matchesAll = true;
            for (const auto &tag : tags) {
                if (!character.tags.contains(tag)) {
                    matchesAll = false;
                    break;
                }
            }
            if (!matchesAll) {
                continue;
            }
        }
        results.push_back(character);
    }
    return results;
}

static QString formatName(const QString &pattern, const QString &name, int index, bool zeroPad, int width) {
    QString formatted = pattern;
    QString indexStr = QString::number(index);
    if (zeroPad) {
        indexStr = QString("%1").arg(index, width, 10, QLatin1Char('0'));
    }
    formatted.replace("%name", name);
    formatted.replace("%index", indexStr);
    return formatted;
}

QVector<Combatant> RosterStore::massAdd(const QString &characterName, int count, const MassAddNaming &naming) const {
    QVector<Combatant> added;
    const auto it = std::find_if(m_characters.begin(), m_characters.end(), [&](const RosterCharacter &c) {
        return c.name.compare(characterName, Qt::CaseInsensitive) == 0;
    });
    if (it == m_characters.end() || count <= 0) {
        return added;
    }

    for (int i = 0; i < count; ++i) {
        Combatant combatant;
        combatant.name = formatName(naming.pattern, it->name, naming.startIndex + i, naming.zeroPad, naming.width);
        combatant.dexMod = it->dexMod;
        combatant.isPC = it->isPC;
        combatant.hp = it->defaultHP;
        combatant.ac = it->defaultAC;
        combatant.notes = it->defaultNotes;
        added.push_back(combatant);
    }
    return added;
}

QVector<Combatant> RosterStore::massAddGroup(const QString &groupName, const MassAddNaming &naming) const {
    QVector<Combatant> combatants;
    const auto it = std::find_if(m_groups.begin(), m_groups.end(), [&](const RosterGroup &group) {
        return group.name.compare(groupName, Qt::CaseInsensitive) == 0;
    });
    if (it == m_groups.end()) {
        return combatants;
    }

    int offset = 0;
    for (const auto &entry : it->entries) {
        MassAddNaming entryNaming = naming;
        entryNaming.startIndex = naming.startIndex + offset;
        const auto added = massAdd(entry.characterName, entry.count, entryNaming);
        for (const auto &combatant : added) {
            combatants.push_back(combatant);
        }
        offset += entry.count;
    }
    return combatants;
}

