#ifndef PRACTICETAB_HPP
#define PRACTICETAB_HPP

// qt
#include <QWidget>

class ChipRow;
class MetronomeEngine;
class PracticeSession;
class QCheckBox;
class QLabel;
class QProgressBar;
class QPushButton;
class StatsStore;
class WeekChart;
class StepperRow;

/// Modo práctica tipo pomodoro, entrenador de velocidad y compases mudos.
class PracticeTab : public QWidget
{
    Q_OBJECT

public:
    PracticeTab(MetronomeEngine *engine, PracticeSession *session, StatsStore *stats, QWidget *parent = nullptr);

    bool practiceEnabled() const;
    void setPracticeEnabled(bool enabled);
    /// Redibuja la sesión (cuenta atrás, fase, valores).
    void refreshSession();
    /// Redibuja el entrenador y los compases mudos.
    void refreshConfig();
    /// Redibuja las estadísticas de constancia.
    void refreshStats();

signals:
    void practiceToggled(bool enabled);
    void toggleRequested();
    void stopRequested();

private:
    MetronomeEngine *engine_;
    PracticeSession *session_;
    StatsStore *stats_;

    QCheckBox *practiceCheck_ = nullptr;
    ChipRow *profiles_ = nullptr;
    StepperRow *practiceMin_ = nullptr;
    StepperRow *restMin_ = nullptr;
    StepperRow *rounds_ = nullptr;
    QLabel *total_ = nullptr;
    QWidget *sessionBody_ = nullptr;
    QLabel *phase_ = nullptr;
    QLabel *countdown_ = nullptr;
    QProgressBar *progress_ = nullptr;
    QPushButton *toggle_ = nullptr;
    QPushButton *stop_ = nullptr;

    QCheckBox *trainerOn_ = nullptr;
    StepperRow *trainerStep_ = nullptr;
    StepperRow *trainerEvery_ = nullptr;
    StepperRow *trainerTarget_ = nullptr;
    QLabel *trainerSummary_ = nullptr;

    QCheckBox *gapOn_ = nullptr;
    StepperRow *gapPlay_ = nullptr;
    StepperRow *gapMute_ = nullptr;
    QCheckBox *gapRandom_ = nullptr;

    QLabel *today_ = nullptr;
    QLabel *streak_ = nullptr;
    WeekChart *week_ = nullptr;
};

#endif // PRACTICETAB_HPP
