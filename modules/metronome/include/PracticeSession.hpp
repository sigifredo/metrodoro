#ifndef PRACTICESESSION_HPP
#define PRACTICESESSION_HPP

// qt
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

class MetronomeEngine;

/**
 * Sesión de práctica tipo pomodoro: rondas de práctica (con el metrónomo
 * sonando) separadas por descansos (en silencio). Termina tras la última ronda
 * de práctica; no hay descanso final.
 */
class PracticeSession : public QObject
{
    Q_OBJECT

public:
    enum class Phase
    {
        Idle,
        Practice,
        Rest,
        Done
    };

    struct Profile
    {
        const char *id;
        int practiceMin;
        int restMin;
        int rounds;
    };

    static constexpr int kMinMinutes = 1;
    static constexpr int kMaxPracticeMin = 60;
    static constexpr int kMaxRestMin = 30;
    static constexpr int kMaxRounds = 8;

    explicit PracticeSession(MetronomeEngine *engine, QObject *parent = nullptr);

    /// Perfiles predefinidos; el último índice libre (`profiles().size()`) es el personalizado.
    static const QList<Profile> &profiles();

    Phase phase() const { return phase_; }
    bool running() const { return running_; }
    int round() const { return round_; }
    int practiceMinutes() const { return practiceMin_; }
    int restMinutes() const { return restMin_; }
    int rounds() const { return rounds_; }
    int remainingSeconds() const;
    int totalMinutes() const;
    /// Índice del perfil que coincide con la configuración actual, o `profiles().size()` si es personalizada.
    int matchingProfile() const;

public slots:
    void toggle();
    void stop();

    void setProfile(int index);
    void changePracticeMinutes(int delta);
    void changeRestMinutes(int delta);
    void changeRounds(int delta);

signals:
    /// Cambió algo que la interfaz debe redibujar.
    void changed();
    void runningChanged(bool running);
    /// Pasó de una fase a otra mientras corría (para avisar al usuario).
    void transitioned();

private:
    void start();
    void pause();
    void resume();
    void enterPractice();
    void enterRest();
    void finish();
    void beginPhase(Phase phase, int seconds);
    void setRunning(bool running);
    void onTick();
    void configChanged();

    MetronomeEngine *engine_;

    Phase phase_ = Phase::Idle;
    bool running_ = false;
    int round_ = 1;

    int practiceMin_ = 15;
    int restMin_ = 3;
    int rounds_ = 3;

    QTimer tick_;
    QElapsedTimer clock_;
    qint64 phaseEndMs_ = 0;
    qint64 pausedLeftMs_ = 0;
};

#endif // PRACTICESESSION_HPP
