// own
#include <ClickConfig.hpp>
#include <TapTempo.hpp>

// std
#include <algorithm>
#include <cmath>

std::optional<int> TapTempo::tap(qint64 nowMs)
{
    if (!taps_.isEmpty() && nowMs - taps_.last() > kResetMs)
        taps_.clear();
    taps_.append(nowMs);
    while (taps_.size() > kMaxTaps)
        taps_.removeFirst();
    if (taps_.size() < 2)
        return std::nullopt;

    const double avgMs = static_cast<double>(taps_.last() - taps_.first()) / (taps_.size() - 1);
    if (avgMs <= 0.0)
        return std::nullopt;
    return std::clamp(static_cast<int>(std::lround(60000.0 / avgMs)), ClickConfig::kMinBpm, ClickConfig::kMaxBpm);
}
