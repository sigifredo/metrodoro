#ifndef TAPTEMPO_HPP
#define TAPTEMPO_HPP

// qt
#include <QList>
#include <QtTypes>

// std
#include <optional>

/// Tap tempo: promedia los últimos toques y olvida la serie si pasan más de cinco segundos sin tocar.
class TapTempo
{
public:
    static constexpr qint64 kResetMs = 5000;
    static constexpr int kMaxTaps = 6;

    /// Registra un toque en el instante `nowMs`; devuelve el BPM estimado a partir del segundo toque.
    std::optional<int> tap(qint64 nowMs);

private:
    QList<qint64> taps_;
};

#endif // TAPTEMPO_HPP
