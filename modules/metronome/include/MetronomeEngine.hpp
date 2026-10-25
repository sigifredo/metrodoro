#ifndef METRONOMEENGINE_HPP
#define METRONOMEENGINE_HPP

// qt
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

#include <memory>

class ClickGenerator;
class QAudioSink;

/**
 * Estado y reglas del metrónomo (BPM, compás, subdivisión, TAP, temporizador)
 * más el control de reproducción. Equivale al `metronome` directive del web.
 */
class MetronomeEngine : public QObject
{
    Q_OBJECT

public:
    static constexpr int kSliderMin = 20;
    static constexpr int kSliderMax = 300;

    explicit MetronomeEngine(QObject *parent = nullptr);
    ~MetronomeEngine() override;

    int bpm() const { return bpm_; }
    int timeInMeasure() const { return timeInMeasure_; }
    int subdivision() const { return dec_; }
    bool shuffle() const { return shuffle_; }
    bool accentFirstBeat() const { return accentFirst_; }
    bool playing() const { return playing_; }
    int currentBeat() const { return currentBeat_; }
    bool timerActive() const { return timerActive_; }
    int timerRemaining() const { return timerRemaining_; }

    /// Nombres de tempo (italiano) que cubren el BPM dado.
    static QStringList tempoLabels(int bpm);

public slots:
    void setBpm(int bpm);
    void startBpmRepeat(int step);
    void stopBpmRepeat();
    void tap();

    void changeTimeInMeasure(int delta);
    void setSubdivision(int dec, bool shuffle);
    void setAccentFirstBeat(bool accent);

    void playPause();
    void stop();

    void setTimerActive(bool active);
    void changeCountdown(int deltaSeconds);

signals:
    void bpmChanged(int bpm);
    void timeInMeasureChanged(int count);
    void subdivisionChanged();
    void accentFirstBeatChanged(bool accent);
    void playingChanged(bool playing);
    void beatChanged(int index);
    void timerChanged();

private:
    void play();
    void resume();
    void pause();
    void restartIfRunning();
    void applyBpmStep(int step);
    void startTimer();
    void stopTimer(bool reset);
    void onTimerTick();

    std::unique_ptr<ClickGenerator> generator_;
    std::unique_ptr<QAudioSink> sink_;

    int bpm_ = 60;
    int timeInMeasure_ = 2;
    int dec_ = 1;
    bool shuffle_ = false;
    bool accentFirst_ = false;

    bool started_ = false;
    bool playing_ = false;
    int currentBeat_ = -1;

    QTimer repeatTimer_;
    int repeatStep_ = 0;
    int repeatCount_ = 0;

    QElapsedTimer tapClock_;
    qint64 lastTapMs_ = -1;

    QTimer countdownTimer_;
    bool timerActive_ = false;
    int timerSeconds_ = 60;
    int timerRemaining_ = 60;
};

#endif // METRONOMEENGINE_HPP
