#ifndef STATSSTORE_HPP
#define STATSSTORE_HPP

// qt
#include <QDate>
#include <QElapsedTimer>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QTimer>

class MetronomeEngine;

/// Días seguidos (hasta hoy, o hasta ayer si hoy aún no se ha practicado) con al menos `minSeconds` de práctica.
int streakDays(const QMap<QDate, int> &seconds, const QDate &today, int minSeconds = 60);

/// Segundos con el metrónomo sonando por día, para mostrar constancia. Todo queda en el equipo.
class StatsStore : public QObject
{
    Q_OBJECT

public:
    explicit StatsStore(QObject *parent = nullptr);

    /// Cuenta el tiempo mientras el motor suene.
    void track(MetronomeEngine *engine);

    const QMap<QDate, int> &days() const { return days_; }
    int secondsOn(const QDate &day) const { return days_.value(day, 0); }
    void add(int seconds, const QDate &day = QDate::currentDate());
    void clear();

signals:
    void changed();

private:
    void flush();
    void commit();

    QSettings settings_;
    QMap<QDate, int> days_;
    QElapsedTimer clock_;
    QTimer flushTimer_;
    qint64 pendingMs_ = 0;
};

#endif // STATSSTORE_HPP
