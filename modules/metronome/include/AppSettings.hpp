#ifndef APPSETTINGS_HPP
#define APPSETTINGS_HPP

// own
#include <ClickConfig.hpp>

// qt
#include <QObject>
#include <QSettings>

/// Lo que se recuerda de la sesión de práctica entre usos.
struct PracticePrefs
{
    bool enabled = false;
    int practiceMin = 15;
    int restMin = 3;
    int rounds = 3;
};

/// Preferencias de la aplicación y último estado del metrónomo, guardados con QSettings.
class AppSettings : public QObject
{
    Q_OBJECT

public:
    explicit AppSettings(QObject *parent = nullptr);

    bool flash() const { return flash_; }
    bool pendulum() const { return pendulum_; }
    bool phaseSound() const { return phaseSound_; }
    float volume() const { return volume_; }
    bool keepAwake() const { return keepAwake_; }
    bool pauseOnUnplug() const { return pauseOnUnplug_; }

    void setFlash(bool on);
    void setPendulum(bool on);
    void setPhaseSound(bool on);
    void setVolume(float volume);
    void setKeepAwake(bool on);
    void setPauseOnUnplug(bool on);
    void reset();

    // Último estado del metrónomo y de la sesión de práctica: la app arranca donde se dejó.
    ClickConfig loadConfig() const;
    void saveConfig(const ClickConfig &config);
    PracticePrefs loadPractice() const;
    void savePractice(const PracticePrefs &prefs);

signals:
    void changed();

private:
    void store();

    QSettings settings_;
    bool flash_ = true;
    bool pendulum_ = false;
    bool phaseSound_ = true;
    float volume_ = 1.0f;
    bool keepAwake_ = true;
    bool pauseOnUnplug_ = true;
};

#endif // APPSETTINGS_HPP
