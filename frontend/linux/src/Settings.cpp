// Settings.cpp
#include "Settings.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonValue>
#include <QTimer>

namespace {
constexpr int kSaveDebounceMs = 400;
}

AppSettings& AppSettings::instance()
{
    // Deliberately leaked. A function-local static QObject would instead be
    // torn down via atexit() *after* QApplication is destroyed (main()'s
    // automatic-storage locals, including QApplication, are destroyed when
    // main() returns, which happens before atexit-registered static-duration
    // destructors run). Qt tolerates that ordering today, but it's a fragile
    // QObject lifetime pattern to rely on. Since this singleton is meant to
    // live for the whole process anyway, skip destruction entirely — the OS
    // reclaims the memory at exit — rather than depend on destructor-order
    // guarantees Qt doesn't make.
    static AppSettings* inst = new AppSettings();
    return *inst;
}

AppSettings::AppSettings()
    : saveTimer_(new QTimer(this))
{
    saveTimer_->setSingleShot(true);
    saveTimer_->setInterval(kSaveDebounceMs);
    connect(saveTimer_, &QTimer::timeout, this, &AppSettings::writeToDisk);
    load();
}

QString AppSettings::configFilePath()
{
    QString base = qEnvironmentVariable("XDG_CONFIG_HOME");
    if (base.isEmpty())
        base = QDir::homePath() + QStringLiteral("/.config");
    return base + QStringLiteral("/ewcalc/config.json");
}

void AppSettings::load()
{
    root_ = QJsonObject{
        { QStringLiteral("version"), kConfigVersion },
        { QStringLiteral("pages"), QJsonObject{} }
    };

    QFile file(configFilePath());
    if (!file.open(QIODevice::ReadOnly))
        return; // No config yet — in-memory defaults stand.

    const auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return; // Corrupt file — ignore, keep in-memory defaults.

    const QJsonObject obj = doc.object();
    // Only version 1 currently exists; a future version bump would migrate
    // old data here before adopting it.
    if (obj.contains(QStringLiteral("pages")))
        root_ = obj;
}

QVariant AppSettings::value(const QString& group, const QString& key, const QVariant& defaultValue) const
{
    const QJsonObject pages = root_.value(QStringLiteral("pages")).toObject();
    const QJsonObject groupObj = pages.value(group).toObject();
    if (!groupObj.contains(key))
        return defaultValue;
    return groupObj.value(key).toVariant();
}

void AppSettings::setValue(const QString& group, const QString& key, const QVariant& value)
{
    QJsonObject pages = root_.value(QStringLiteral("pages")).toObject();
    QJsonObject groupObj = pages.value(group).toObject();
    groupObj.insert(key, QJsonValue::fromVariant(value));
    pages.insert(group, groupObj);
    root_.insert(QStringLiteral("pages"), pages);
    root_.insert(QStringLiteral("version"), kConfigVersion);

    dirty_ = true;
    saveTimer_->start(); // (Re)start the debounce window.
}

void AppSettings::flush()
{
    if (!dirty_)
        return;
    saveTimer_->stop();
    writeToDisk();
}

void AppSettings::writeToDisk()
{
    const QString path = configFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        file.write(QJsonDocument(root_).toJson(QJsonDocument::Indented));
    dirty_ = false;
}

void AppSettings::resetAll()
{
    root_ = QJsonObject{
        { QStringLiteral("version"), kConfigVersion },
        { QStringLiteral("pages"), QJsonObject{} }
    };
    dirty_ = true;
    saveTimer_->stop();
    writeToDisk();
    emit resetRequested();
}
