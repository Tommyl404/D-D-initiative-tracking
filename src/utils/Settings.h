#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace initiative {

class Settings {
public:
    explicit Settings(std::filesystem::path filePath);

    void load();
    void save() const;

    void setTheme(const std::string& theme);
    const std::string& theme() const;

    void setAutosaveIntervalMinutes(int minutes);
    int autosaveIntervalMinutes() const;

    void setLastFiles(std::vector<std::string> files);
    const std::vector<std::string>& lastFiles() const;

    void setStreamerMode(bool enabled);
    bool streamerMode() const;

private:
    std::filesystem::path filePath_;
    std::string theme_{"dark"};
    int autosaveIntervalMinutes_{2};
    std::vector<std::string> lastFiles_;
    bool streamerMode_{false};
};

}
