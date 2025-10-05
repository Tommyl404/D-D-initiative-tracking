#pragma once

#include "core/Combatant.h"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace initiative {

struct Character {
    std::string name;
    int dexMod{0};
    bool isPC{false};
    std::vector<std::string> tags;
    int defaultHP{0};
    int defaultAC{0};
    std::string defaultNotes;
};

struct GroupEntry {
    std::string character;
    int count{1};
};

struct Group {
    std::string name;
    std::vector<GroupEntry> entries;
};

struct NamingOptions {
    std::string pattern{"%name %index"};
    int startIndex{1};
    bool zeroPad{false};
    int width{2};
};

class RosterStore {
public:
    void loadCharacters(const std::filesystem::path& path);
    void loadGroups(const std::filesystem::path& path);

    const std::vector<Character>& characters() const;
    const std::vector<Group>& groups() const;
    const NamingOptions& namingOptions() const;

    std::vector<Character> filterByTag(const std::string& tag) const;
    std::optional<Character> findCharacter(const std::string& name) const;
    std::vector<Combatant> instantiateGroup(const std::string& groupName, int& nextId) const;

private:
    Combatant instantiateCharacter(const Character& character, const std::string& nameOverride, int& nextId) const;

    std::vector<Character> characters_;
    std::vector<Group> groups_;
    NamingOptions namingOptions_;
    std::unordered_map<std::string, Character> characterLookup_;
};

}
