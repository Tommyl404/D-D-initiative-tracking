#pragma once

#include <QObject>
#include <QSettings>

class Settings : public QObject {
    Q_OBJECT
public:
    explicit Settings(QObject *parent = nullptr);

    int autosaveIntervalMinutes() const;
    void setAutosaveIntervalMinutes(int minutes);

    QString theme() const;
    void setTheme(const QString &theme);

    bool streamerMode() const;
    void setStreamerMode(bool enabled);

    QString lastEncounterPath() const;
    void setLastEncounterPath(const QString &path);

private:
    QSettings m_settings;
};

