// own
#include <ClickGenerator.hpp>

// std
#include <algorithm>

ClickGenerator::ClickGenerator(QObject *parent)
    : QIODevice(parent)
{
}

void ClickGenerator::setConfig(const ClickConfig &config)
{
    QMutexLocker lock(&mutex_);
    live_ = config;
}

void ClickGenerator::setSequence(const QList<SequenceStep> &steps, bool loop)
{
    QMutexLocker lock(&mutex_);
    sequence_ = steps;
    loop_ = loop;
}

void ClickGenerator::clearSequence()
{
    QMutexLocker lock(&mutex_);
    sequence_.reset();
}

void ClickGenerator::restart()
{
    {
        QMutexLocker lock(&mutex_);
        pending_.clear();
    }
    restartRequested_ = true;
}

std::deque<Pulse> ClickGenerator::takePulses()
{
    QMutexLocker lock(&mutex_);
    return std::exchange(pending_, {});
}

ClickGenerator::Snapshot ClickGenerator::snapshot()
{
    QMutexLocker lock(&mutex_);
    return {live_, sequence_, loop_};
}

void ClickGenerator::resetState()
{
    const Snapshot s = snapshot();
    cfg_ = s.live;
    planner_.reset(cfg_);
    barInfo_ = {false, false, cfg_.bpm};
    samplesUntilNext_ = 0.0;
    frame_ = 0;
    slot_ = 0;
    beat_ = 0;
    needBar_ = true;
    bar_ = 0;
    segment_ = 0;
    segBars_ = 0;
    segStarted_ = false;
    inSequence_ = false;
    finished_ = false;
    polyActive_ = false;
    polyIdx_ = 0;
    polyCountdown_ = 0.0;
    voiceTable_.fill(nullptr);
    voicePos_.fill(0);
    nextVoice_ = 0;
}

qint64 ClickGenerator::readData(char *data, qint64 maxlen)
{
    if (restartRequested_.exchange(false))
        resetState();

    auto *out = reinterpret_cast<int16_t *>(data);
    const qint64 frames = maxlen / static_cast<qint64>(sizeof(int16_t));
    for (qint64 i = 0; i < frames; ++i)
        out[i] = renderFrame();
    return frames * static_cast<qint64>(sizeof(int16_t));
}

int16_t ClickGenerator::renderFrame()
{
    if (!finished_ && samplesUntilNext_ <= 0.0)
    {
        startNote();
        samplesUntilNext_ += kSampleRate * 60.0 / barInfo_.bpm / subdivisionSlots(cfg_.subdivision);
    }
    if (polyActive_ && cfg_.poly.enabled)
    {
        if (polyCountdown_ <= 0.0 && polyIdx_ < cfg_.poly.beats)
        {
            play(cfg_.poly.sound, ClickSynth::Mid, cfg_.poly.volume);
            {
                QMutexLocker lock(&mutex_);
                pending_.push_back({frame_, polyIdx_, bar_, barInfo_.bpm, false, false, segment_, false, true});
            }
            ++polyIdx_;
            polyCountdown_ += polyPeriod_;
        }
        polyCountdown_ -= 1.0;
    }
    samplesUntilNext_ -= 1.0;
    ++frame_;

    int sum = 0;
    for (int v = 0; v < kVoices; ++v)
    {
        const ClickSynth::Table *table = voiceTable_[v];
        if (!table)
            continue;
        const int pos = voicePos_[v];
        if (pos >= static_cast<int>(table->size()))
        {
            voiceTable_[v] = nullptr;
            continue;
        }
        sum += static_cast<int>((*table)[pos] * voiceGain_[v]);
        voicePos_[v] = pos + 1;
    }
    return static_cast<int16_t>(std::clamp(sum, -32767, 32767));
}

void ClickGenerator::play(ClickSound sound, int level, float gain)
{
    const int v = nextVoice_;
    nextVoice_ = (nextVoice_ + 1) % kVoices;
    voiceTable_[v] = &ClickSynth::tables(sound)[level];
    voicePos_[v] = 0;
    voiceGain_[v] = gain;
}

void ClickGenerator::startNote()
{
    const Snapshot s = snapshot();
    if (s.sequence)
    {
        inSequence_ = true;
    }
    else
    {
        if (inSequence_)
        {
            // Se salió del repertorio en caliente: se sigue con la configuración libre.
            inSequence_ = false;
            ClickConfig fresh = s.live;
            fresh.countInBars = 0;
            planner_.reset(fresh);
            needBar_ = true;
            beat_ = 0;
            slot_ = 0;
        }
        if (s.live.beats != cfg_.beats || s.live.subdivision != cfg_.subdivision || s.live.unit != cfg_.unit)
        {
            needBar_ = true;
            beat_ = 0;
            slot_ = 0;
        }
        cfg_ = s.live;
        if (planner_.rebase(s.live.bpm))
            barInfo_.bpm = s.live.bpm;
    }

    if (needBar_ && !beginBar(s.sequence, s.loop))
        return;

    const qint64 noteFrame = frame_;
    const QList<bool> &mask = subdivisionMask(cfg_.subdivision);
    if (slot_ == 0)
    {
        const Accent accent = beat_ < cfg_.accents.size() ? cfg_.accents[beat_] : Accent::Weak;
        if (!barInfo_.muted && accent != Accent::Mute)
        {
            const int level = accent == Accent::Strong   ? ClickSynth::High
                              : accent == Accent::Medium ? ClickSynth::Mid
                                                         : ClickSynth::Low;
            play(cfg_.sound, level, accentGain(accent));
        }
        QMutexLocker lock(&mutex_);
        pending_.push_back({noteFrame, beat_, bar_, barInfo_.bpm, barInfo_.muted, barInfo_.countIn, segment_, false});
    }
    else if (mask[slot_] && !barInfo_.muted)
    {
        play(cfg_.sound, ClickSynth::Low, cfg_.subVolume * accentGain(Accent::Weak));
    }

    ++slot_;
    if (slot_ >= mask.size())
    {
        slot_ = 0;
        ++beat_;
        if (beat_ >= cfg_.beats)
        {
            beat_ = 0;
            needBar_ = true;
        }
    }
}

bool ClickGenerator::beginBar(const std::optional<QList<SequenceStep>> &sequence, bool loop)
{
    if (sequence)
    {
        const QList<SequenceStep> &seq = *sequence;
        if (!segStarted_)
        {
            segment_ = 0;
            segStarted_ = true;
            startSegment(seq[0]);
        }
        else if (segBars_ >= seq[std::min<int>(segment_, seq.size() - 1)].bars && !barInfo_.countIn)
        {
            if (segment_ + 1 >= seq.size() && !loop)
            {
                finished_ = true;
                QMutexLocker lock(&mutex_);
                pending_.push_back({frame_, -1, bar_, barInfo_.bpm, true, false, segment_, true});
                return false;
            }
            segment_ = (segment_ + 1) % seq.size();
            startSegment(seq[segment_]);
        }
        cfg_ = seq[segment_].config;
    }

    barInfo_ = planner_.next(cfg_);
    if (sequence && !barInfo_.countIn)
        ++segBars_;
    if (!sequence)
        segStarted_ = false;
    bar_ = planner_.playedBars();
    needBar_ = false;

    // La segunda capa arranca con el compás y calla en la cuenta previa y en los compases mudos.
    polyActive_ = cfg_.poly.enabled && !barInfo_.countIn && !barInfo_.muted;
    polyIdx_ = 0;
    polyCountdown_ = 0.0;
    polyPeriod_ = kSampleRate * 60.0 / barInfo_.bpm * cfg_.beats / std::max(cfg_.poly.beats, 1);
    return true;
}

void ClickGenerator::startSegment(const SequenceStep &step)
{
    cfg_ = step.config;
    planner_.reset(cfg_);
    segBars_ = 0;
    barInfo_ = {false, false, cfg_.bpm};
}
