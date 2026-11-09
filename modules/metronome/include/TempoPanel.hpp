#ifndef TEMPOPANEL_HPP
#define TEMPOPANEL_HPP

// qt
#include <QVariantAnimation>
#include <QWidget>

class AppSettings;
class BeatRowWidget;
class ClickableLabel;
class MetronomeEngine;
class PendulumWidget;
class PlayButton;
class PracticeSession;
class QFrame;
class QLabel;
class QSlider;
struct LiveState;

/// Columna del tempo: cifra de BPM, deslizador, tiempos del compás, TAP y play. Es lo que se toca mientras se estudia.
class TempoPanel : public QWidget
{
    Q_OBJECT

public:
    TempoPanel(MetronomeEngine *engine, PracticeSession *session, AppSettings *settings, QWidget *parent = nullptr);

    void setPlaying(bool playing);
    void setPracticeEnabled(bool enabled);
    /// Redibuja lo que depende de la configuración, del repertorio o de la sesión.
    void refresh();

signals:
    void playClicked();
    void bannerClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void onPulse(const LiveState &live);
    void updateBpm();
    void updateBanner();
    void askBpm();

    MetronomeEngine *engine_;
    PracticeSession *session_;
    AppSettings *settings_;
    bool practiceEnabled_ = false;

    QFrame *banner_ = nullptr;
    QLabel *bannerText_ = nullptr;
    QLabel *bannerClock_ = nullptr;
    QLabel *tempoLabel_ = nullptr;
    ClickableLabel *bpmLabel_ = nullptr;
    QLabel *noteLabel_ = nullptr;
    QSlider *slider_ = nullptr;
    PendulumWidget *pendulum_ = nullptr;
    BeatRowWidget *beats_ = nullptr;
    PlayButton *playButton_ = nullptr;

    QVariantAnimation flash_;
    qreal flashAlpha_ = 0.0;
};

#endif // TEMPOPANEL_HPP
