#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

// qt
#include <QWidget>

class BeatDotsWidget;
class MetronomeEngine;
class NoteSelectorWidget;
class PlayButton;
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
    void updateTimer();

    MetronomeEngine *engine_ = nullptr;

    QLabel *bpmLabel_ = nullptr;
    QLabel *tempoLabel_ = nullptr;
    QSlider *slider_ = nullptr;
    BeatDotsWidget *dots_ = nullptr;
    PlayButton *playButton_ = nullptr;
    QCheckBox *accentCheck_ = nullptr;
    QLabel *measureLabel_ = nullptr;
    NoteSelectorWidget *notes_ = nullptr;
    QCheckBox *timerCheck_ = nullptr;
    QLabel *timerLabel_ = nullptr;
};

#endif // MAINWINDOW_HPP
