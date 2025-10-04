#include "EncounterStore.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
constexpr int kSchemaVersion = 2;

QJsonObject toJson(const Combatant &combatant) {
    QJsonObject obj;
    obj["id"] = combatant.id;
    obj["name"] = combatant.name;
    obj["initiative"] = combatant.initiative;
    obj["dexMod"] = combatant.dexMod;
    obj["isPC"] = combatant.isPC;
    obj["conscious"] = combatant.conscious;
    obj["hp"] = combatant.hp;
    obj["ac"] = combatant.ac;
    QJsonObject ds;
    ds["successes"] = combatant.deathSaves.successes;
    ds["failures"] = combatant.deathSaves.failures;
    ds["dead"] = combatant.deathSaves.dead;
    ds["stable"] = combatant.deathSaves.stable;
    obj["deathSaves"] = ds;
    QJsonArray conditions;
    for (const auto &condition : combatant.conditions) {
        QJsonObject conditionObj;
        conditionObj["name"] = condition.name;
        conditionObj["remainingRounds"] = condition.remainingRounds;
        conditions.push_back(conditionObj);
    }
    obj["conditions"] = conditions;
    obj["notes"] = combatant.notes;
    return obj;
}

Combatant combatantFromJson(const QJsonObject &obj) {
    Combatant combatant;
    combatant.id = obj.value("id").toInt();
    combatant.name = obj.value("name").toString();
    combatant.initiative = obj.value("initiative").toInt();
    combatant.dexMod = obj.value("dexMod").toInt();
    combatant.isPC = obj.value("isPC").toBool();
    combatant.conscious = obj.value("conscious").toBool(true);
    combatant.hp = obj.value("hp").toInt();
    combatant.ac = obj.value("ac").toInt();
    const auto ds = obj.value("deathSaves").toObject();
    combatant.deathSaves.successes = ds.value("successes").toInt();
    combatant.deathSaves.failures = ds.value("failures").toInt();
    combatant.deathSaves.dead = ds.value("dead").toBool();
    combatant.deathSaves.stable = ds.value("stable").toBool();
    for (const auto &conditionValue : obj.value("conditions").toArray()) {
        const auto conditionObj = conditionValue.toObject();
        Condition condition;
        condition.name = conditionObj.value("name").toString();
        condition.remainingRounds = conditionObj.value("remainingRounds").toInt();
        combatant.conditions.push_back(condition);
    }
    combatant.notes = obj.value("notes").toString();
    return combatant;
}
}

EncounterStore::EncounterStore(QObject *parent)
    : QObject(parent) {}

void EncounterStore::setFilePath(QString path) {
    m_filePath = std::move(path);
}

bool EncounterStore::load(TurnManager &manager, int &round, int &turnIndex) const {
    if (m_filePath.isEmpty()) {
        return false;
    }
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const auto data = file.readAll();
    return deserialize(data, manager, round, turnIndex);
}

bool EncounterStore::save(const TurnManager &manager, int round, int turnIndex) const {
    if (m_filePath.isEmpty()) {
        return false;
    }
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(serialize(manager, round, turnIndex));
    return true;
}

QByteArray EncounterStore::serialize(const TurnManager &manager, int round, int turnIndex) {
    QJsonObject root;
    root["schema"] = kSchemaVersion;
    root["round"] = round;
    root["turnIndex"] = turnIndex;
    QJsonArray combatants;
    for (const auto &combatant : manager.combatants()) {
        combatants.push_back(toJson(combatant));
    }
    root["combatants"] = combatants;
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

bool EncounterStore::deserialize(const QByteArray &data, TurnManager &manager, int &round, int &turnIndex) {
    const auto doc = QJsonDocument::fromJson(data);
    const auto root = doc.object();
    if (root.value("schema").toInt() != kSchemaVersion) {
        return false;
    }
    round = root.value("round").toInt(1);
    turnIndex = root.value("turnIndex").toInt(0);

    TurnManager::CombatantList list;
    for (const auto &value : root.value("combatants").toArray()) {
        list.push_back(combatantFromJson(value.toObject()));
    }
    manager.setCombatants(list);
    return true;
}

