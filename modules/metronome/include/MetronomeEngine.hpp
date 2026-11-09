#ifndef METRONOMEENGINE_HPP
#define METRONOMEENGINE_HPP

// own
#include <ClickConfig.hpp>
#include <ClickGenerator.hpp>
#include <TapTempo.hpp>

// qt
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

// std
#include <deque>
#include <functional>
#include <memory>
#include <optional>

class QAudioSink;

/// Lo que está sonando ahora; la interfaz lo usa para los puntos, el péndulo y el destello.
struct LiveState
{
    int beat = -1;
    int bar = 0;
    int bpm = 0;
    bool muted = false;
    bool countIn = false;
    /// Contador de pulsos: cambia en cada tiempo aunque el resto sea igual.
    qint64 seq = 0;
};

/// Un repertorio en marcha: los pasos, sus nombres y cuál suena.
struct SetlistRun
{
    QString name;
    QStringList names;
    QList<SequenceStep> steps;
    int segment = 0;
};

/**
 * Estado y reglas del metrónomo (configuración, TAP, repertorio) más el control de reproducción.
 * Los pulsos se entregan cuando la posición de reproducción alcanza su muestra, para que puntos,
 * péndulo y destello coincidan con lo que se oye.
 */
class MetronomeEngine : public QObject
{
    Q_OBJECT

public:
    explicit MetronomeEngine(QObject *parent = nullptr);
    ~MetronomeEngine() override;

    const ClickConfig &config() const { return config_; }
    bool running() const { return running_; }
    /// Pulso actual de la segunda capa (-1 si no suena).
    int polyBeat() const { return polyBeat_; }
    const LiveState &live() const { return live_; }
    const std::optional<SetlistRun> &setlistRun() const { return setlistRun_; }
    float volume() const { return volume_; }

    /// Tempo que se muestra: el del pulso durante el entrenador, el configurado si está parado.
    /// Aplica una transformación a la configuración actual.
    void edit(const std::function<ClickConfig(const ClickConfig &)> &transform);

    int shownBpm() const { return running_ && live_.bpm > 0 ? live_.bpm : config_.bpm; }

public slots:
    /// Cambia la configuración. Si sonaba un repertorio, se sale de él y se sigue con lo editado.
    void setConfig(const ClickConfig &config);
    void setBpm(int bpm);
    void startBpmRepeat(int step);
    void stopBpmRepeat();
    void tap();
    void setVolume(float volume);

    void start();
    void stop();
    void toggle();

    bool startSetlist(const QString &name, const QStringList &names, const QList<SequenceStep> &steps, bool loop);
    void exitSetlist();

signals:
    void configChanged();
    void runningChanged(bool running);
    void pulse(const LiveState &live);
    void polyPulse(int index);
    void setlistChanged();

private:
    void applyConfig(const ClickConfig &config);
    void pollPulses();
    void handlePulse(const Pulse &pulse);

    std::unique_ptr<ClickGenerator> generator_;
    std::unique_ptr<QAudioSink> sink_;

    ClickConfig config_;
    bool running_ = false;
    LiveState live_;
    std::optional<SetlistRun> setlistRun_;
    float volume_ = 1.0f;

    std::deque<Pulse> due_;
    QTimer pollTimer_;
    qint64 pulseSeq_ = 0;
    int polyBeat_ = -1;

    QTimer repeatTimer_;
    int repeatStep_ = 0;
    int repeatCount_ = 0;

    QElapsedTimer clock_;
    TapTempo tapTempo_;
};

#endif // METRONOMEENGINE_HPP
