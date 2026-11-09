// own
#include <ClickSynth.hpp>
#include <PhaseChime.hpp>

// qt
#include <QAudioFormat>
#include <QAudioSink>
#include <QBuffer>
#include <QMediaDevices>
#include <QTimer>

void PhaseChime::play(bool rising, int notes, float volume)
{
    const ClickSynth::Table pcm = ClickSynth::chime(rising, notes);

    QAudioFormat format;
    format.setSampleRate(ClickSynth::kSampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    auto *buffer = new QBuffer(this);
    buffer->setData(QByteArray(reinterpret_cast<const char *>(pcm.data()), static_cast<qsizetype>(pcm.size() * sizeof(int16_t))));
    buffer->open(QIODevice::ReadOnly);

    auto *sink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format, this);
    sink->setVolume(volume);
    sink->start(buffer);

    const int ms = static_cast<int>(pcm.size() * 1000 / ClickSynth::kSampleRate) + 300;
    QTimer::singleShot(ms, this, [sink, buffer]()
                       {
                           sink->stop();
                           sink->deleteLater();
                           buffer->deleteLater(); });
}
