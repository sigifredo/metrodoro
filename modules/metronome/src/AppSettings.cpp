// own
#include <AppSettings.hpp>

// qt
#include <QJsonDocument>

// std
#include <algorithm>

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
{
    flash_ = settings_.value("flash", true).toBool();
    pendulum_ = settings_.value("pendulum", false).toBool();
    phaseSound_ = settings_.value("phaseSound", true).toBool();
    volume_ = std::clamp(settings_.value("volume", 1.0).toFloat(), 0.0f, 1.0f);
}

void AppSettings::store()
{
    settings_.setValue("flash", flash_);
    settings_.setValue("pendulum", pendulum_);
    settings_.setValue("phaseSound", phaseSound_);
    settings_.setValue("volume", volume_);
    emit changed();
}

void AppSettings::setFlash(bool on)
{
    flash_ = on;
    store();
}

void AppSettings::setPendulum(bool on)
{
    pendulum_ = on;
    store();
}

void AppSettings::setPhaseSound(bool on)
{
    phaseSound_ = on;
    store();
}

void AppSettings::setVolume(float volume)
{
    volume_ = std::clamp(volume, 0.0f, 1.0f);
    store();
}

void AppSettings::reset()
{
    flash_ = true;
    pendulum_ = false;
    phaseSound_ = true;
    volume_ = 1.0f;
    store();
}

ClickConfig AppSettings::loadConfig() const
{
    const auto doc = QJsonDocument::fromJson(settings_.value("config").toByteArray());
    return doc.isObject() ? ClickConfig::fromJson(doc.object()) : ClickConfig();
}

void AppSettings::saveConfig(const ClickConfig &config)
{
    settings_.setValue("config", QJsonDocument(config.toJson()).toJson(QJsonDocument::Compact));
}

PracticePrefs AppSettings::loadPractice() const
{
    PracticePrefs p;
    p.enabled = settings_.value("practice/enabled", p.enabled).toBool();
    p.practiceMin = settings_.value("practice/minutes", p.practiceMin).toInt();
    p.restMin = settings_.value("practice/rest", p.restMin).toInt();
    p.rounds = settings_.value("practice/rounds", p.rounds).toInt();
    return p;
}

void AppSettings::savePractice(const PracticePrefs &p)
{
    settings_.setValue("practice/enabled", p.enabled);
    settings_.setValue("practice/minutes", p.practiceMin);
    settings_.setValue("practice/rest", p.restMin);
    settings_.setValue("practice/rounds", p.rounds);
}
