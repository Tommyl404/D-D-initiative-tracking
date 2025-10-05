#include "storage/EncounterStore.h"

#include "storage/SimpleJson.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace initiative {

namespace {
using simplejson::JsonValue;

Combatant parseCombatant(const JsonValue& value) {
    const auto& object = value.asObject();
    Combatant combatant;
    combatant.id = object.at("id").asInt();
    combatant.name = object.at("name").asString();
    combatant.initiative = object.at("initiative").asInt();
    combatant.dexMod = object.at("dexMod").asInt();
    combatant.isPC = object.at("isPC").asBool();
    combatant.conscious = object.at("conscious").asBool();
    combatant.hp = object.at("hp").asInt();
    combatant.ac = object.at("ac").asInt();
    if (value.contains("notes")) {
        combatant.notes = value.at("notes").asString();
    }
    if (value.contains("deathSaves")) {
        const auto& savesObj = value.at("deathSaves").asObject();
        combatant.deathSaves.successes = savesObj.at("successes").asInt();
        combatant.deathSaves.failures = savesObj.at("failures").asInt();
        combatant.deathSaves.dead = savesObj.at("dead").asBool();
        combatant.deathSaves.stable = savesObj.at("stable").asBool();
    }
    if (value.contains("conditions")) {
        for (const auto& conditionValue : value.at("conditions").asArray()) {
            const auto& conditionObj = conditionValue.asObject();
            combatant.conditions.push_back({conditionObj.at("name").asString(),
                                            static_cast<int>(conditionObj.at("remainingRounds").asInt())});
        }
    }
    return combatant;
}

JsonValue serializeCombatant(const Combatant& combatant) {
    JsonValue::Object object;
    object.emplace("id", JsonValue(static_cast<int64_t>(combatant.id)));
    object.emplace("name", JsonValue(combatant.name));
    object.emplace("initiative", JsonValue(static_cast<int64_t>(combatant.initiative)));
    object.emplace("dexMod", JsonValue(static_cast<int64_t>(combatant.dexMod)));
    object.emplace("isPC", JsonValue(combatant.isPC));
    object.emplace("conscious", JsonValue(combatant.conscious));
    object.emplace("hp", JsonValue(static_cast<int64_t>(combatant.hp)));
    object.emplace("ac", JsonValue(static_cast<int64_t>(combatant.ac)));
    object.emplace("notes", JsonValue(combatant.notes));

    JsonValue::Object savesObj;
    savesObj.emplace("successes", JsonValue(static_cast<int64_t>(combatant.deathSaves.successes)));
    savesObj.emplace("failures", JsonValue(static_cast<int64_t>(combatant.deathSaves.failures)));
    savesObj.emplace("dead", JsonValue(combatant.deathSaves.dead));
    savesObj.emplace("stable", JsonValue(combatant.deathSaves.stable));
    object.emplace("deathSaves", JsonValue(std::move(savesObj)));

    JsonValue::Array conditionsArray;
    for (const auto& condition : combatant.conditions) {
        JsonValue::Object conditionObj;
        conditionObj.emplace("name", JsonValue(condition.name));
        conditionObj.emplace("remainingRounds", JsonValue(static_cast<int64_t>(condition.remainingRounds)));
        conditionsArray.emplace_back(JsonValue(std::move(conditionObj)));
    }
    object.emplace("conditions", JsonValue(std::move(conditionsArray)));

    return JsonValue(std::move(object));
}
}

EncounterData EncounterStore::loadFromFile(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to open encounter file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    JsonValue data = simplejson::parse(buffer.str());
    const auto& object = data.asObject();

    EncounterData encounter;
    encounter.schema = static_cast<int>(object.at("schema").asInt());
    if (encounter.schema != 2) {
        throw std::runtime_error("Unsupported encounter schema");
    }
    encounter.round = static_cast<int>(object.at("round").asInt());
    encounter.turnIndex = static_cast<std::size_t>(object.at("turnIndex").asInt());
    if (data.contains("combatants")) {
        for (const auto& value : data.at("combatants").asArray()) {
            encounter.combatants.push_back(parseCombatant(value));
        }
    }
    return encounter;
}

void EncounterStore::saveToFile(const EncounterData& encounter, const std::filesystem::path& path) {
    JsonValue::Object object;
    object.emplace("schema", JsonValue(static_cast<int64_t>(encounter.schema)));
    object.emplace("round", JsonValue(static_cast<int64_t>(encounter.round)));
    object.emplace("turnIndex", JsonValue(static_cast<int64_t>(encounter.turnIndex)));

    JsonValue::Array combatantArray;
    for (const auto& combatant : encounter.combatants) {
        combatantArray.emplace_back(serializeCombatant(combatant));
    }
    object.emplace("combatants", JsonValue(std::move(combatantArray)));

    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Unable to write encounter file: " + path.string());
    }
    output << simplejson::stringify(JsonValue(std::move(object)), 2);
}

}
