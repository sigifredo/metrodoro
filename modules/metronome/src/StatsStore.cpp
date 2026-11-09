// own
#include <MetronomeEngine.hpp>
#include <StatsStore.hpp>

namespace
{
    constexpr int kKeepDays = 90;
    constexpr int kFlushMs = 10000;
}

int streakDays(const QMap<QDate, int> &seconds, const QDate &today, int minSeconds)
{
    QDate day = seconds.value(today, 0) >= minSeconds ? today : today.addDays(-1);
    int n = 0;
    while (seconds.value(day, 0) >= minSeconds)
    {
        ++n;
        day = day.addDays(-1);
    }
    return n;
}

StatsStore::StatsStore(QObject *parent)
    : QObject(parent)
{
    settings_.beginGroup(QStringLiteral("stats"));
    for (const QString &key : settings_.childKeys())
    {
        const QDate day = QDate::fromString(key, Qt::ISODate);
        if (day.isValid())
            days_[day] = settings_.value(key).toInt();
    }
    settings_.endGroup();

    flushTimer_.setInterval(kFlushMs);
    connect(&flushTimer_, &QTimer::timeout, this, &StatsStore::flush);
}

void StatsStore::track(MetronomeEngine *engine)
{
    connect(engine, &MetronomeEngine::runningChanged, this, [this](bool running)
            {
                if (running)
                {
                    clock_.start();
                    flushTimer_.start();
                }
                else
                {
                    pendingMs_ += clock_.elapsed();
                    flushTimer_.stop();
                    commit();
                } });
}

void StatsStore::flush()
{
    pendingMs_ += clock_.restart();
    commit();
}

void StatsStore::commit()
{
    const int seconds = static_cast<int>(pendingMs_ / 1000);
    if (seconds > 0)
    {
        pendingMs_ -= seconds * 1000LL;
        add(seconds);
    }
}

void StatsStore::add(int seconds, const QDate &day)
{
    if (seconds <= 0)
        return;
    days_[day] += seconds;
    // Se guardan solo los últimos 90 días.
    const QDate cut = day.addDays(-kKeepDays);
    for (auto it = days_.begin(); it != days_.end();)
        it = it.key() < cut ? days_.erase(it) : std::next(it);

    settings_.beginGroup(QStringLiteral("stats"));
    settings_.remove(QString());
    for (auto it = days_.cbegin(); it != days_.cend(); ++it)
        settings_.setValue(it.key().toString(Qt::ISODate), it.value());
    settings_.endGroup();
    emit changed();
}

void StatsStore::clear()
{
    days_.clear();
    pendingMs_ = 0;
    settings_.beginGroup(QStringLiteral("stats"));
    settings_.remove(QString());
    settings_.endGroup();
    emit changed();
}
