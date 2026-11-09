// own
#include <SleepInhibitor.hpp>

#ifdef METRODORO_HAS_DBUS
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#endif

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
    active_ = active;
}
