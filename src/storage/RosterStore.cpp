#include "storage/RosterStore.h"

#include "storage/SimpleJson.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace initiative {

namespace {
using simplejson::JsonValue;

Character parseCharacter(const JsonValue& value) {
    const auto& object = value.asObject();
    Character character;
    character.name = object.at("name").asString();
    character.dexMod = static_cast<int>(object.at("dexMod").asInt());
    character.isPC = object.at("isPC").asBool();
    if (value.contains("tags")) {
        for (const auto& tag : value.at("tags").asArray()) {
            character.tags.push_back(tag.asString());
        }
    }
    if (value.contains("defaultHP")) {
        character.defaultHP = static_cast<int>(value.at("defaultHP").asInt());
    }
    if (value.contains("defaultAC")) {
        character.defaultAC = static_cast<int>(value.at("defaultAC").asInt());
    }
    if (value.contains("defaultNotes")) {
        character.defaultNotes = value.at("defaultNotes").asString();
    }
    return character;
}

Group parseGroup(const JsonValue& value) {
    const auto& object = value.asObject();
    Group group;
    group.name = object.at("name").asString();
    if (value.contains("entries")) {
        for (const auto& entryValue : value.at("entries").asArray()) {
            const auto& entryObj = entryValue.asObject();
            GroupEntry entry;
            entry.character = entryObj.at("character").asString();
            if (entryValue.contains("count")) {
                entry.count = static_cast<int>(entryObj.at("count").asInt());
            }
            group.entries.push_back(std::move(entry));
        }
    }
    return group;
}

NamingOptions parseNamingOptions(const JsonValue& value) {
    const auto& object = value.asObject();
    NamingOptions options;
    if (value.contains("pattern")) {
        options.pattern = value.at("pattern").asString();
    }
    if (value.contains("startIndex")) {
        options.startIndex = static_cast<int>(value.at("startIndex").asInt());
    }
    if (value.contains("zeroPad")) {
        options.zeroPad = value.at("zeroPad").asBool();
    }
    if (value.contains("width")) {
        options.width = static_cast<int>(value.at("width").asInt());
    }
    return options;
}

std::string formatIndex(int value, const NamingOptions& options) {
    if (!options.zeroPad) {
        return std::to_string(value);
    }
    const int width = options.width > 0 ? options.width : 2;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(width) << value;
    return oss.str();
}

std::string applyPattern(const NamingOptions& options, const std::string& baseName, int index) {
    std::string result = options.pattern;
    const std::string indexValue = formatIndex(index, options);

    std::size_t pos = 0;
    while ((pos = result.find("%name", pos)) != std::string::npos) {
        result.replace(pos, 5, baseName);
        pos += baseName.size();
    }

    pos = 0;
    while ((pos = result.find("%index", pos)) != std::string::npos) {
        result.replace(pos, 6, indexValue);
        pos += indexValue.size();
    }

    return result;
}
}

void RosterStore::loadCharacters(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to open characters file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    JsonValue data = simplejson::parse(buffer.str());
    const auto& object = data.asObject();

    if (data.contains("schema") && data.at("schema").asInt() != 2) {
        throw std::runtime_error("Unsupported character schema");
    }

    characters_.clear();
    characterLookup_.clear();
    if (data.contains("characters")) {
        for (const auto& value : data.at("characters").asArray()) {
            auto character = parseCharacter(value);
            characterLookup_.emplace(character.name, character);
            characters_.push_back(std::move(character));
        }
    }
}

void RosterStore::loadGroups(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to open groups file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    JsonValue data = simplejson::parse(buffer.str());
    const auto& object = data.asObject();

    if (data.contains("schema") && data.at("schema").asInt() != 2) {
        throw std::runtime_error("Unsupported group schema");
    }

    groups_.clear();
    if (data.contains("groups")) {
        for (const auto& value : data.at("groups").asArray()) {
            groups_.push_back(parseGroup(value));
        }
    }
    if (data.contains("naming")) {
        namingOptions_ = parseNamingOptions(data.at("naming"));
    }
}

const std::vector<Character>& RosterStore::characters() const {
    return characters_;
}

const std::vector<Group>& RosterStore::groups() const {
    return groups_;
}

const NamingOptions& RosterStore::namingOptions() const {
    return namingOptions_;
}

std::vector<Character> RosterStore::filterByTag(const std::string& tag) const {
    std::vector<Character> matches;
    for (const auto& character : characters_) {
        if (std::find(character.tags.begin(), character.tags.end(), tag) != character.tags.end()) {
            matches.push_back(character);
        }
    }
    return matches;
}

std::optional<Character> RosterStore::findCharacter(const std::string& name) const {
    if (auto it = characterLookup_.find(name); it != characterLookup_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Combatant> RosterStore::instantiateGroup(const std::string& groupName, int& nextId) const {
    auto groupIt = std::find_if(groups_.begin(), groups_.end(), [&](const Group& group) {
        return group.name == groupName;
    });

    if (groupIt == groups_.end()) {
        throw std::runtime_error("Group not found: " + groupName);
    }

    std::vector<Combatant> result;
    for (const auto& entry : groupIt->entries) {
        auto character = findCharacter(entry.character);
        if (!character) {
            throw std::runtime_error("Character not found: " + entry.character);
        }
        int indexValue = namingOptions_.startIndex;
        for (int offset = 0; offset < entry.count; ++offset) {
            const int index = indexValue++;
            const std::string name = applyPattern(namingOptions_, character->name, index);
            result.push_back(instantiateCharacter(*character, name, nextId));
        }
    }
    return result;
}

Combatant RosterStore::instantiateCharacter(const Character& character, const std::string& nameOverride, int& nextId) const {
    Combatant combatant;
    combatant.id = nextId++;
    combatant.name = nameOverride;
    combatant.dexMod = character.dexMod;
    combatant.isPC = character.isPC;
    combatant.conscious = true;
    combatant.hp = character.defaultHP;
    combatant.ac = character.defaultAC;
    combatant.notes = character.defaultNotes;
    return combatant;
}

}
