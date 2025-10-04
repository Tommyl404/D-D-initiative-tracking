#include "Settings.h"

#include <QCoreApplication>

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings("OpenAI", "InitiativeTracker") {
    m_settings.setFallbacksEnabled(true);
}

int Settings::autosaveIntervalMinutes() const {
    return m_settings.value("autosaveInterval", 2).toInt();
}

void Settings::setAutosaveIntervalMinutes(int minutes) {
    m_settings.setValue("autosaveInterval", minutes);
}

QString Settings::theme() const {
    return m_settings.value("theme", "light").toString();
}

void Settings::setTheme(const QString &theme) {
    m_settings.setValue("theme", theme);
}

bool Settings::streamerMode() const {
    return m_settings.value("streamerMode", false).toBool();
}

void Settings::setStreamerMode(bool enabled) {
    m_settings.setValue("streamerMode", enabled);
}

QString Settings::lastEncounterPath() const {
    return m_settings.value("lastEncounterPath").toString();
}

void Settings::setLastEncounterPath(const QString &path) {
    m_settings.setValue("lastEncounterPath", path);
}

