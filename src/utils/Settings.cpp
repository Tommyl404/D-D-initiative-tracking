#include "utils/Settings.h"

#include "storage/SimpleJson.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace initiative {

Settings::Settings(std::filesystem::path filePath)
    : filePath_(std::move(filePath)) {}

void Settings::load() {
    std::ifstream input(filePath_);
    if (!input) {
        return;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    simplejson::JsonValue data = simplejson::parse(buffer.str());
    const auto& object = data.asObject();

    if (data.contains("theme")) {
        theme_ = data.at("theme").asString();
    }
    if (data.contains("autosaveMinutes")) {
        autosaveIntervalMinutes_ = static_cast<int>(data.at("autosaveMinutes").asInt());
    }
    if (data.contains("lastFiles")) {
        lastFiles_.clear();
        for (const auto& entry : data.at("lastFiles").asArray()) {
            lastFiles_.push_back(entry.asString());
        }
    }
    if (data.contains("streamerMode")) {
        streamerMode_ = data.at("streamerMode").asBool();
    }
}

void Settings::save() const {
    if (!filePath_.parent_path().empty()) {
        std::filesystem::create_directories(filePath_.parent_path());
    }

    simplejson::JsonValue::Object object;
    object.emplace("theme", simplejson::JsonValue(theme_));
    object.emplace("autosaveMinutes", simplejson::JsonValue(static_cast<int64_t>(autosaveIntervalMinutes_)));

    simplejson::JsonValue::Array filesArray;
    for (const auto& file : lastFiles_) {
        filesArray.emplace_back(simplejson::JsonValue(file));
    }
    object.emplace("lastFiles", simplejson::JsonValue(std::move(filesArray)));
    object.emplace("streamerMode", simplejson::JsonValue(streamerMode_));

    std::ofstream output(filePath_);
    if (!output) {
        throw std::runtime_error("Unable to write settings file: " + filePath_.string());
    }
    output << simplejson::stringify(simplejson::JsonValue(std::move(object)), 2);
}

void Settings::setTheme(const std::string& theme) {
    theme_ = theme;
}

const std::string& Settings::theme() const {
    return theme_;
}

void Settings::setAutosaveIntervalMinutes(int minutes) {
    autosaveIntervalMinutes_ = std::max(1, minutes);
}

int Settings::autosaveIntervalMinutes() const {
    return autosaveIntervalMinutes_;
}

void Settings::setLastFiles(std::vector<std::string> files) {
    lastFiles_ = std::move(files);
}

const std::vector<std::string>& Settings::lastFiles() const {
    return lastFiles_;
}

void Settings::setStreamerMode(bool enabled) {
    streamerMode_ = enabled;
}

bool Settings::streamerMode() const {
    return streamerMode_;
}

}
