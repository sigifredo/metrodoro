#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

// own
#include <AppSettings.hpp>

// qt
#include <QWidget>

class MetronomeEngine;
class MetronomeTab;
class PhaseChime;
class PracticeSession;
class PracticeTab;
class QTabWidget;
class SettingsTab;
class StatsStore;
class QTimer;
class TempoPanel;

/**
 * Ventana de escritorio. A la izquierda va lo que se toca mientras se estudia (tempo, tiempos, TAP y play);
 * a la derecha, en pestañas, lo que se cambia poco. Todo se maneja también con el teclado.
 */
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void buildUi();
    void buildShortcuts();
    void onConfigChanged();
    void onSessionChanged();
    void updatePlayButton();
    /// Con el modo práctica activo manda la sesión; si no, el motor.
    void togglePlayback();
    void stopAll();
    void persistPractice();

    AppSettings *settings_ = nullptr;
    MetronomeEngine *engine_ = nullptr;
    PracticeSession *session_ = nullptr;
    StatsStore *stats_ = nullptr;
    PhaseChime *chime_ = nullptr;

    TempoPanel *tempo_ = nullptr;
    QTabWidget *tabs_ = nullptr;
    MetronomeTab *metronomeTab_ = nullptr;
    PracticeTab *practiceTab_ = nullptr;
    SettingsTab *settingsTab_ = nullptr;

    PracticePrefs savedPractice_;
    bool practiceEnabled_ = false;
};

#endif // MAINWINDOW_HPP
