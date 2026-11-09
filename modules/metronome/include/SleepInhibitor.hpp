#ifndef SLEEPINHIBITOR_HPP
#define SLEEPINHIBITOR_HPP

// qt
#include <QObject>

#ifdef METRODORO_HAS_DBUS
#include <QDBusUnixFileDescriptor>
#endif

#ifdef Q_OS_MACOS
#include <cstdint>
#endif

/// Evita que el equipo se suspenda mientras suena el metrónomo. Usa logind por D-Bus en Linux, IOKit en macOS y SetThreadExecutionState en Windows.
class SleepInhibitor : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    ~SleepInhibitor() override;

    void setActive(bool active);
    bool active() const { return active_; }

private:
    bool active_ = false;
#ifdef METRODORO_HAS_DBUS
    QDBusUnixFileDescriptor lock_;
#endif
#ifdef Q_OS_MACOS
    std::uint32_t assertion_ = 0; // IOPMAssertionID
#endif
};

#endif // SLEEPINHIBITOR_HPP
