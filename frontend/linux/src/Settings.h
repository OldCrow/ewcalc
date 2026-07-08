#pragma once
/// @file Settings.h
/// @brief Lightweight JSON-backed persistence for user inputs across sessions.
///
/// Config file location: $XDG_CONFIG_HOME/ewcalc/config.json, falling back to
/// ~/.config/ewcalc/config.json when XDG_CONFIG_HOME is unset. Values are
/// grouped by page name (e.g. "Propagation") and keyed by field name
/// (e.g. "distance_km"). Writes are coalesced with a short debounce timer and
/// are always flushed on application quit (see main.cpp).

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariant>

class QTimer;

class AppSettings : public QObject
{
    Q_OBJECT
public:
    /// Returns the process-wide settings instance.
    static AppSettings& instance();

    /// Returns the stored value for group/key, or defaultValue if absent.
    QVariant value(const QString& group, const QString& key, const QVariant& defaultValue) const;

    /// Stores value for group/key and schedules a debounced write to disk.
    void setValue(const QString& group, const QString& key, const QVariant& value);

    /// Writes any pending changes to disk immediately. Call before quitting.
    void flush();

    /// Clears all stored values, both in memory and on disk, and emits
    /// resetRequested().
    void resetAll();

    /// Full path to the config file, respecting $XDG_CONFIG_HOME.
    static QString configFilePath();

signals:
    /// Emitted after resetAll() clears the store.
    void resetRequested();

private:
    AppSettings();

    void load();
    void writeToDisk();

    QJsonObject root_;
    QTimer* saveTimer_;
    bool dirty_ = false;

    static constexpr int kConfigVersion = 1;
};
