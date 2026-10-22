#ifndef CLICKGENERATOR_HPP
#define CLICKGENERATOR_HPP

// qt
#include <QIODevice>

// std
#include <atomic>

/**
 * Sintetiza el metrónomo como PCM Int16 mono, muestra a muestra.
 * Port de `scheduleNote`/`nextNote` del metrónomo web: cada nota es un seno
 * con envolvente lineal de 10 ms. Los parámetros son atómicos porque
 * `readData` se ejecuta en el hilo de audio.
 */
class ClickGenerator : public QIODevice
{
    Q_OBJECT

public:
    static constexpr int kSampleRate = 44100;

    explicit ClickGenerator(QObject *parent = nullptr);

    void setBpm(int bpm) { bpm_ = bpm; }
    void setSubdivision(int dec) { dec_ = dec; }
    void setTimeInMeasure(int count) { timeInMeasure_ = count; }
    void setShuffle(bool shuffle) { shuffle_ = shuffle; }
    void setAccentFirstBeat(bool accent) { accentFirst_ = accent; }

    /// Reinicia la secuencia: el siguiente clic es el primer tiempo.
    void restart();

    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override { return 1 << 16; }

signals:
    /// Tiempo de compás (0-based) de la nota que acaba de empezar.
    void beat(int index);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override { return 0; }

private:
    struct Tone
    {
        double freq;
        double volume;
    };

    void startNote();

    std::atomic<int> bpm_{60};
    std::atomic<int> dec_{1};
    std::atomic<int> timeInMeasure_{2};
    std::atomic<bool> shuffle_{false};
    std::atomic<bool> accentFirst_{false};
    std::atomic<bool> restartRequested_{true};

    double samplesUntilNext_ = 0.0;
    int noteIndex_ = 0;
    bool clickActive_ = false;
    qint64 clickPos_ = 0;
    Tone clickTone_{0.0, 0.0};
};

#endif // CLICKGENERATOR_HPP
