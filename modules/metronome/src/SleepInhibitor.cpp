// own
#include <SleepInhibitor.hpp>

#ifdef Q_OS_MACOS
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef METRODORO_HAS_DBUS
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#endif

SleepInhibitor::~SleepInhibitor()
{
    setActive(false);
}

void SleepInhibitor::setActive(bool active)
{
    if (active == active_)
        return;
#ifdef METRODORO_HAS_DBUS
    if (active)
    {
        // El bloqueo dura mientras se conserve el descriptor devuelto por logind.
        QDBusInterface manager(QStringLiteral("org.freedesktop.login1"), QStringLiteral("/org/freedesktop/login1"),
                               QStringLiteral("org.freedesktop.login1.Manager"), QDBusConnection::systemBus());
        if (!manager.isValid())
            return;
        const QDBusReply<QDBusUnixFileDescriptor> reply = manager.call(QStringLiteral("Inhibit"), QStringLiteral("sleep"),
                                                                       QStringLiteral("Metrodoro"), QStringLiteral("Metronome playing"),
                                                                       QStringLiteral("block"));
        if (!reply.isValid())
            return;
        lock_ = reply.value();
    }
    else
    {
        lock_ = QDBusUnixFileDescriptor();
    }
#endif
#ifdef Q_OS_MACOS
    if (active)
    {
        IOPMAssertionID id = 0;
        const CFStringRef reason = CFSTR("Metrodoro: metronome playing");
        if (IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleSystemSleep, kIOPMAssertionLevelOn, reason, &id) !=
            kIOReturnSuccess)
            return;
        assertion_ = id;
    }
    else if (assertion_ != 0)
    {
        IOPMAssertionRelease(assertion_);
        assertion_ = 0;
    }
#endif
#ifdef Q_OS_WIN
    SetThreadExecutionState(active ? (ES_CONTINUOUS | ES_SYSTEM_REQUIRED) : ES_CONTINUOUS);
#endif
    active_ = active;
}
