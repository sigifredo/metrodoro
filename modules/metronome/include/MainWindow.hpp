#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

// qt
#include <QWidget>

class BeatDotsWidget;
class MetronomeEngine;
class NoteSelectorWidget;
class PlayButton;
class PracticeSession;
class QComboBox;
class QCheckBox;
class QLabel;
class QSlider;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void buildUi();
    void updateBpm();
    void updatePractice();

    MetronomeEngine *engine_ = nullptr;

    QLabel *bpmLabel_ = nullptr;
    QLabel *tempoLabel_ = nullptr;
    QSlider *slider_ = nullptr;
    BeatDotsWidget *dots_ = nullptr;
    PlayButton *playButton_ = nullptr;
    QCheckBox *accentCheck_ = nullptr;
    QLabel *measureLabel_ = nullptr;
    NoteSelectorWidget *notes_ = nullptr;

    PracticeSession *session_ = nullptr;
    QCheckBox *practiceCheck_ = nullptr;
    QComboBox *profileCombo_ = nullptr;
    QWidget *practiceBody_ = nullptr;
    QLabel *phaseLabel_ = nullptr;
    QLabel *countdownLabel_ = nullptr;
    QLabel *practiceMinLabel_ = nullptr;
    QLabel *restMinLabel_ = nullptr;
    QLabel *roundsLabel_ = nullptr;
    QLabel *totalLabel_ = nullptr;
};

#endif // MAINWINDOW_HPP
