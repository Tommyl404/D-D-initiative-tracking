#pragma once

#include "core/Combatant.h"

#include <filesystem>
#include <vector>

namespace initiative {

struct EncounterData {
    int schema{2};
    int round{1};
    std::size_t turnIndex{0};
    std::vector<Combatant> combatants;
};

class EncounterStore {
public:
    static EncounterData loadFromFile(const std::filesystem::path& path);
    static void saveToFile(const EncounterData& encounter, const std::filesystem::path& path);
};

}
