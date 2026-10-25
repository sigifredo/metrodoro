// own
#include <ClickGenerator.hpp>
#include <MetronomeEngine.hpp>

// qt
#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>

// std
#include <algorithm>
#include <cmath>

namespace
{
    struct TempoRange
    {
        int min;
        int max;
        const char *name;
    };

    constexpr TempoRange kTempos[] = {
        {10, 39, "Larghissimo"}, {40, 60, "Largo"}, {52, 68, "Lento"},
        {60, 80, "Adagio"}, {76, 100, "Andante"}, {88, 112, "Moderato"},
        {100, 128, "Allegretto"}, {112, 160, "Allegro"}, {138, 142, "Vivace"},
        {140, 200, "Presto"}, {188, 260, "Prestissimo"}};

    constexpr int kButtonMinBpm = 10;
    constexpr int kButtonMaxBpm = 260;
    constexpr int kTapMinBpm = 20;
    constexpr int kTapMaxBpm = 260;
    constexpr int kMinTimeInMeasure = 2;
    constexpr int kMaxTimeInMeasure = 12;
    constexpr int kRepeatSlowMs = 300;
    constexpr int kRepeatMediumMs = 100;
    constexpr int kRepeatFastMs = 50;
}

MetronomeEngine::MetronomeEngine(QObject *parent)
    : QObject(parent), generator_(std::make_unique<ClickGenerator>())
{
    QAudioFormat format;
    format.setSampleRate(ClickGenerator::kSampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    sink_ = std::make_unique<QAudioSink>(QMediaDevices::defaultAudioOutput(), format);
    // Búfer corto para que el parpadeo de los tiempos no se retrase.
    sink_->setBufferSize(ClickGenerator::kSampleRate / 20 * 2);

    generator_->open(QIODevice::ReadOnly);
    generator_->setBpm(bpm_);
    generator_->setSubdivision(dec_);
    generator_->setTimeInMeasure(timeInMeasure_);

    connect(generator_.get(), &ClickGenerator::beat, this, [this](int index)
            {
                currentBeat_ = index;
                emit beatChanged(index);
            });

    repeatTimer_.setInterval(kRepeatSlowMs);
    connect(&repeatTimer_, &QTimer::timeout, this, [this]()
            {
                applyBpmStep(repeatStep_);
                if (repeatCount_ == 5)
                    repeatTimer_.setInterval(kRepeatMediumMs);
                else if (repeatCount_ == 20)
                    repeatTimer_.setInterval(kRepeatFastMs);
            });
}

MetronomeEngine::~MetronomeEngine()
{
    sink_->stop();
}

QStringList MetronomeEngine::tempoLabels(int bpm)
{
    QStringList labels;
    for (const auto &tempo : kTempos)
    {
        if (tempo.min <= bpm && bpm <= tempo.max)
            labels << QString::fromLatin1(tempo.name);
    }
    return labels;
}

/* ---- BPM ----------------------------------------------------------------- */

void MetronomeEngine::setBpm(int bpm)
{
    bpm = std::clamp(bpm, kButtonMinBpm, kSliderMax);
    if (bpm == bpm_)
        return;
    bpm_ = bpm;
    generator_->setBpm(bpm_);
    emit bpmChanged(bpm_);
}

void MetronomeEngine::applyBpmStep(int step)
{
    const int value = bpm_ + step;
    if (value < kButtonMinBpm || value > kButtonMaxBpm)
        return;
    ++repeatCount_;
    setBpm(value);
}

void MetronomeEngine::startBpmRepeat(int step)
{
    applyBpmStep(step);
    if (repeatTimer_.isActive())
        return;
    repeatStep_ = step;
    repeatTimer_.setInterval(kRepeatSlowMs);
    repeatTimer_.start();
}

void MetronomeEngine::stopBpmRepeat()
{
    repeatTimer_.stop();
    repeatCount_ = 0;
}

void MetronomeEngine::tap()
{
    if (!tapClock_.isValid())
        tapClock_.start();

    const qint64 now = tapClock_.elapsed();
    if (lastTapMs_ < 0)
    {
        lastTapMs_ = now;
        return;
    }

    const double seconds = static_cast<double>(now - lastTapMs_) / 1000.0;
    lastTapMs_ = now;
    if (seconds <= 0.0)
        return;

    const int value = static_cast<int>(std::ceil(60.0 / seconds));
    setBpm(std::clamp(value, kTapMinBpm, kTapMaxBpm));
}

/* ---- Compás y subdivisión ------------------------------------------------- */

void MetronomeEngine::changeTimeInMeasure(int delta)
{
    const int value = timeInMeasure_ + delta;
    if (value < kMinTimeInMeasure || value > kMaxTimeInMeasure)
        return;
    timeInMeasure_ = value;
    generator_->setTimeInMeasure(value);
    emit timeInMeasureChanged(value);
    restartIfRunning();
}

void MetronomeEngine::setSubdivision(int dec, bool shuffle)
{
    dec_ = dec;
    shuffle_ = shuffle;
    generator_->setSubdivision(dec);
    generator_->setShuffle(shuffle);
    emit subdivisionChanged();
    restartIfRunning();
}

void MetronomeEngine::setAccentFirstBeat(bool accent)
{
    accentFirst_ = accent;
    generator_->setAccentFirstBeat(accent);
    emit accentFirstBeatChanged(accent);
}

/* ---- Reproducción -------------------------------------------------------- */

void MetronomeEngine::playPause()
{
    if (!started_)
        play();
    else if (!playing_)
        resume();
    else
        pause();
}

void MetronomeEngine::play()
{
    generator_->restart();
    sink_->start(generator_.get());
    started_ = true;
    playing_ = true;
    emit playingChanged(true);
}

void MetronomeEngine::resume()
{
    generator_->restart();
    sink_->start(generator_.get());
    playing_ = true;
    emit playingChanged(true);
}

void MetronomeEngine::pause()
{
    sink_->stop();
    playing_ = false;
    emit playingChanged(false);
}

void MetronomeEngine::stop()
{
    sink_->stop();
    started_ = false;
    playing_ = false;
    currentBeat_ = -1;
    emit playingChanged(false);
    emit beatChanged(-1);
}

void MetronomeEngine::restartIfRunning()
{
    if (playing_)
    {
        stop();
        play();
    }
    else if (started_)
    {
        stop();
    }
}
