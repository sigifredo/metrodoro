// own
#include <MetronomeEngine.hpp>

// qt
#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>

// std
#include <algorithm>

namespace
{
    constexpr int kRepeatSlowMs = 300;
    constexpr int kRepeatMediumMs = 100;
    constexpr int kRepeatFastMs = 50;
    constexpr int kPollMs = 4;
}

MetronomeEngine::MetronomeEngine(QObject *parent)
    : QObject(parent), generator_(std::make_unique<ClickGenerator>())
{
    QAudioFormat format;
    format.setSampleRate(ClickGenerator::kSampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    sink_ = std::make_unique<QAudioSink>(QMediaDevices::defaultAudioOutput(), format);
    // Búfer corto para que los indicadores no se retrasen respecto al sonido.
    sink_->setBufferSize(ClickGenerator::kSampleRate / 20 * 2);

    generator_->open(QIODevice::ReadOnly);
    generator_->setConfig(config_);
    clock_.start();

    pollTimer_.setInterval(kPollMs);
    pollTimer_.setTimerType(Qt::PreciseTimer);
    connect(&pollTimer_, &QTimer::timeout, this, &MetronomeEngine::pollPulses);

    repeatTimer_.setInterval(kRepeatSlowMs);
    connect(&repeatTimer_, &QTimer::timeout, this, [this]()
            {
                setBpm(config_.bpm + repeatStep_);
                ++repeatCount_;
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

/* ---- Configuración ------------------------------------------------------- */

void MetronomeEngine::applyConfig(const ClickConfig &config)
{
    config_ = config;
    generator_->setConfig(config_);
    emit configChanged();
}

void MetronomeEngine::setConfig(const ClickConfig &config)
{
    if (config == config_)
        return;
    if (setlistRun_)
        exitSetlist();
    applyConfig(config);
}

void MetronomeEngine::edit(const std::function<ClickConfig(const ClickConfig &)> &transform)
{
    setConfig(transform(config_));
}

void MetronomeEngine::setBpm(int bpm)
{
    setConfig(config_.withBpm(bpm));
}

void MetronomeEngine::startBpmRepeat(int step)
{
    setBpm(config_.bpm + step);
    if (repeatTimer_.isActive())
        return;
    repeatStep_ = step;
    repeatCount_ = 0;
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
    if (const auto bpm = tapTempo_.tap(clock_.elapsed()))
        setBpm(*bpm);
}

void MetronomeEngine::setVolume(float volume)
{
    volume_ = std::clamp(volume, 0.0f, 1.0f);
    sink_->setVolume(volume_);
}

/* ---- Reproducción -------------------------------------------------------- */

void MetronomeEngine::start()
{
    if (running_)
        return;
    generator_->setConfig(config_);
    generator_->restart();
    due_.clear();
    live_ = {};
    sink_->start(generator_.get());
    running_ = true;
    pollTimer_.start();
    emit runningChanged(true);
    emit pulse(live_);
}

void MetronomeEngine::stop()
{
    if (!running_)
        return;
    sink_->stop();
    pollTimer_.stop();
    due_.clear();
    running_ = false;
    live_ = {};
    emit runningChanged(false);
    emit pulse(live_);
}

void MetronomeEngine::toggle()
{
    if (running_)
        stop();
    else
        start();
}

/* ---- Pulsos -------------------------------------------------------------- */

void MetronomeEngine::pollPulses()
{
    for (const Pulse &p : generator_->takePulses())
        due_.push_back(p);

    const qint64 played = sink_->processedUSecs() * ClickGenerator::kSampleRate / 1000000;
    while (!due_.empty() && due_.front().frame <= played)
    {
        const Pulse p = due_.front();
        due_.pop_front();
        handlePulse(p);
        if (!running_)
            return;
    }
}

void MetronomeEngine::handlePulse(const Pulse &p)
{
    if (p.end)
    {
        setlistRun_.reset();
        generator_->clearSequence();
        emit setlistChanged();
        stop();
        return;
    }

    live_ = {p.beat, p.bar, p.bpm, p.muted, p.countIn, ++pulseSeq_};
    if (setlistRun_ && setlistRun_->segment != p.segment && p.segment < setlistRun_->steps.size())
    {
        setlistRun_->segment = p.segment;
        // Se muestra la configuración de la pieza en curso sin salir del repertorio.
        applyConfig(setlistRun_->steps[p.segment].config);
        emit setlistChanged();
    }
    emit pulse(live_);
}

/* ---- Repertorios --------------------------------------------------------- */

bool MetronomeEngine::startSetlist(const QString &name, const QStringList &names, const QList<SequenceStep> &steps, bool loop)
{
    if (steps.isEmpty())
        return false;
    stop();
    generator_->setSequence(steps, loop);
    setlistRun_ = SetlistRun{name, names, steps, 0};
    applyConfig(steps.first().config);
    emit setlistChanged();
    start();
    return true;
}

void MetronomeEngine::exitSetlist()
{
    if (!setlistRun_)
        return;
    generator_->clearSequence();
    setlistRun_.reset();
    emit setlistChanged();
}
