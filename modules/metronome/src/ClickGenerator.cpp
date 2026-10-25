// own
#include <ClickGenerator.hpp>

// qt
#include <QMetaObject>

// std
#include <algorithm>
#include <cmath>
#include <numbers>

namespace
{
    constexpr double kStrongFreq = 987.767;
    constexpr double kWeakFreq = 783.991;
    constexpr double kMasterVolume = 7.0;
    constexpr double kAttackSeconds = 0.01;
    constexpr double kNoteSeconds = 0.05;
    constexpr double kStrongVolume = 1.0;
    constexpr double kWeakVolume = 0.5;
    constexpr double kSubdivisionVolume = 0.03;
}

ClickGenerator::ClickGenerator(QObject *parent)
    : QIODevice(parent)
{
}

void ClickGenerator::restart()
{
    restartRequested_ = true;
}

void ClickGenerator::startNote()
{
    const int dec = dec_;
    const int count = timeInMeasure_;
    const int n = noteIndex_;

    clickActive_ = true;
    clickPos_ = 0;

    if (n == 0)
    {
        clickTone_ = accentFirst_ ? Tone{kStrongFreq, kStrongVolume}
                                  : Tone{kWeakFreq, kWeakVolume};
    }
    else if (n % dec == 0)
    {
        clickTone_ = {kWeakFreq, kWeakVolume};
    }
    else if ((n - 1) % 3 == 0 && shuffle_)
    {
        clickActive_ = false;
    }
    else
    {
        // En negras (dec == 1) la subdivisión no existe: silencio.
        clickTone_ = {kWeakFreq, dec == 1 ? 0.0 : kSubdivisionVolume};
    }

    const int beatIndex = n / dec;
    QMetaObject::invokeMethod(this, [this, beatIndex]()
                              { emit beat(beatIndex); }, Qt::QueuedConnection);

    noteIndex_ = (n + 1) % (dec * count);
}

qint64 ClickGenerator::readData(char *data, qint64 maxlen)
{
    if (restartRequested_.exchange(false))
    {
        samplesUntilNext_ = 0.0;
        noteIndex_ = 0;
        clickActive_ = false;
    }

    auto *out = reinterpret_cast<qint16 *>(data);
    const qint64 frames = maxlen / static_cast<qint64>(sizeof(qint16));

    for (qint64 i = 0; i < frames; ++i)
    {
        if (samplesUntilNext_ <= 0.0)
        {
            startNote();
            samplesUntilNext_ += kSampleRate * 60.0 / bpm_ / dec_;
        }
        samplesUntilNext_ -= 1.0;

        double sample = 0.0;
        if (clickActive_)
        {
            const double t = static_cast<double>(clickPos_) / kSampleRate;
            if (t < kNoteSeconds)
            {
                const double envelope = std::max(0.0, 1.0 - t / kAttackSeconds);
                const double gain = clickTone_.volume * kMasterVolume * envelope;
                sample = std::clamp(gain * std::sin(2.0 * std::numbers::pi * clickTone_.freq * t), -1.0, 1.0);
                ++clickPos_;
            }
            else
            {
                clickActive_ = false;
            }
        }
        out[i] = static_cast<qint16>(sample * 32767.0);
    }

    return frames * static_cast<qint64>(sizeof(qint16));
}
