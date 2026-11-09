#ifndef CLICKGENERATOR_HPP
#define CLICKGENERATOR_HPP

// own
#include <BarPlanner.hpp>
#include <ClickConfig.hpp>
#include <ClickSynth.hpp>

// qt
#include <QIODevice>
#include <QList>
#include <QMutex>

// std
#include <array>
#include <atomic>
#include <deque>
#include <optional>

/// Un paso de un repertorio: se toca `config` durante `bars` compases.
struct SequenceStep
{
    ClickConfig config;
    int bars = 1;
};

/**
 * Pulso de un tiempo (no de cada subdivisión) con la muestra exacta en la que suena.
 * `end` marca el final de un repertorio que no se repite.
 */
struct Pulse
{
    qint64 frame = 0;
    int beat = -1;
    int bar = 0;
    int bpm = 0;
    bool muted = false;
    bool countIn = false;
    int segment = 0;
    bool end = false;
};

/**
 * Sintetiza el metrónomo como PCM Int16 mono, muestra a muestra. Un contador de fracción en `double`
 * hace que cada clic caiga en su muestra aunque el intervalo no sea entero, y hasta 16 voces solapadas
 * permiten subdivisiones rápidas. Entrenador, mudos, cuenta previa y avance de repertorio se deciden en
 * el límite de compás. `readData` corre en el hilo de audio; el resto de la interfaz habla con él a
 * través de un candado que solo se toma una vez por nota.
 */
class ClickGenerator : public QIODevice
{
    Q_OBJECT

public:
    static constexpr int kSampleRate = ClickSynth::kSampleRate;

    explicit ClickGenerator(QObject *parent = nullptr);

    /// Configuración libre en vivo: se aplica en la siguiente nota.
    void setConfig(const ClickConfig &config);
    /// Si hay secuencia, manda sobre la configuración libre; `clearSequence` vuelve al modo libre en caliente.
    void setSequence(const QList<SequenceStep> &steps, bool loop);
    void clearSequence();

    /// Reinicia la reproducción: el siguiente clic es el primer tiempo del primer compás.
    void restart();

    /// Pulsos generados desde la última llamada.
    std::deque<Pulse> takePulses();

    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override { return 1 << 16; }

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override { return 0; }

private:
    static constexpr int kVoices = 16;

    struct Snapshot
    {
        ClickConfig live;
        std::optional<QList<SequenceStep>> sequence;
        bool loop = true;
    };

    Snapshot snapshot();
    void resetState();
    int16_t renderFrame();
    void startNote();
    bool beginBar(const std::optional<QList<SequenceStep>> &sequence, bool loop);
    void startSegment(const SequenceStep &step);
    void play(ClickSound sound, int level, float gain);

    QMutex mutex_;
    ClickConfig live_;
    std::optional<QList<SequenceStep>> sequence_;
    bool loop_ = true;
    std::deque<Pulse> pending_;
    std::atomic<bool> restartRequested_{true};

    // Estado exclusivo del hilo de audio.
    BarPlanner planner_;
    ClickConfig cfg_;
    BarPlanner::Bar barInfo_;
    double samplesUntilNext_ = 0.0;
    qint64 frame_ = 0;
    int slot_ = 0;
    int beat_ = 0;
    bool needBar_ = true;
    int bar_ = 0;
    int segment_ = 0;
    int segBars_ = 0;
    bool segStarted_ = false;
    bool inSequence_ = false;
    bool finished_ = false;

    std::array<const ClickSynth::Table *, kVoices> voiceTable_{};
    std::array<int, kVoices> voicePos_{};
    std::array<float, kVoices> voiceGain_{};
    int nextVoice_ = 0;
};

#endif // CLICKGENERATOR_HPP
