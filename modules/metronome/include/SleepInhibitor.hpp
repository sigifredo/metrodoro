#ifndef SLEEPINHIBITOR_HPP
#define SLEEPINHIBITOR_HPP

// qt
#include <QObject>

#ifdef METRODORO_HAS_DBUS
#include <QDBusUnixFileDescriptor>
#endif

/// Evita que el equipo se suspenda mientras suena el metrónomo. Solo tiene efecto en Linux (logind por D-Bus).
class SleepInhibitor : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    void setActive(bool active);
    bool active() const { return active_; }

private:
    bool active_ = false;
#ifdef METRODORO_HAS_DBUS
    QDBusUnixFileDescriptor lock_;
#endif
};

#endif // SLEEPINHIBITOR_HPP
