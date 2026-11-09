// own
#include <AppSettings.hpp>
#include <BeatRowWidget.hpp>
#include <Controls.hpp>
#include <MetronomeEngine.hpp>
#include <PendulumWidget.hpp>
#include <PlayButton.hpp>
#include <PracticeSession.hpp>
#include <RepeatButton.hpp>
#include <TempoPanel.hpp>
#include <Texts.hpp>

// qt
#include <QEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QStyle>
#include <QVBoxLayout>

// std
#include <algorithm>

namespace
{
    QString clock(int seconds)
    {
        return QStringLiteral("%1:%2").arg(seconds / 60, 2, 10, QLatin1Char('0')).arg(seconds % 60, 2, 10, QLatin1Char('0'));
    }
}

TempoPanel::TempoPanel(MetronomeEngine *engine, PracticeSession *session, AppSettings *settings, QWidget *parent)
    : QWidget(parent), engine_(engine), session_(session), settings_(settings)
{
    auto *column = new QVBoxLayout(this);
    column->setContentsMargins(24, 20, 24, 20);
    column->setSpacing(10);

    // Aviso de práctica o repertorio en curso (se pulsa para ir a la pestaña de práctica).
    banner_ = new QFrame(this);
    banner_->setObjectName(QStringLiteral("banner"));
    banner_->setCursor(Qt::PointingHandCursor);
    auto *bannerRow = new QHBoxLayout(banner_);
    bannerRow->setContentsMargins(14, 8, 14, 8);
    bannerText_ = new QLabel(banner_);
    bannerText_->setObjectName(QStringLiteral("bannerText"));
    bannerClock_ = new QLabel(banner_);
    bannerClock_->setObjectName(QStringLiteral("bannerClock"));
    bannerRow->addWidget(bannerText_, 1);
    bannerRow->addWidget(bannerClock_);
    column->addWidget(banner_);
    banner_->installEventFilter(this);

    tempoLabel_ = new QLabel(this);
    tempoLabel_->setObjectName(QStringLiteral("tempo"));
    tempoLabel_->setAlignment(Qt::AlignCenter);
    column->addWidget(tempoLabel_);

    bpmLabel_ = new ClickableLabel(this);
    bpmLabel_->setObjectName(QStringLiteral("bpm"));
    bpmLabel_->setAlignment(Qt::AlignCenter);
    bpmLabel_->setTextFormat(Qt::RichText);
    bpmLabel_->setCursor(Qt::IBeamCursor);
    bpmLabel_->setToolTip(txt("Haz clic para escribir el tempo", "Click to type the tempo", "Cliquez pour saisir le tempo"));
    column->addWidget(bpmLabel_);

    noteLabel_ = new QLabel(this);
    noteLabel_->setObjectName(QStringLiteral("note"));
    noteLabel_->setAlignment(Qt::AlignCenter);
    column->addWidget(noteLabel_);

    // −5 −1 [slider] +1 +5
    auto *bpmRow = new QHBoxLayout;
    bpmRow->setSpacing(8);
    slider_ = new QSlider(Qt::Horizontal, this);
    slider_->setRange(ClickConfig::kMinBpm, ClickConfig::kMaxBpm);
    slider_->setToolTip(QStringLiteral("Tempo (Ctrl+↑ / Ctrl+↓)"));
    for (int step : {-5, -1})
    {
        auto *button = new RepeatButton(QStringLiteral("\u2212%1").arg(-step), this);
        button->setFixedSize(40, 40);
        bpmRow->addWidget(button);
        connect(button, &QAbstractButton::pressed, this, [this, step]()
                { engine_->startBpmRepeat(step); });
        connect(button, &QAbstractButton::released, engine_, &MetronomeEngine::stopBpmRepeat);
    }
    bpmRow->addWidget(slider_, 1);
    for (int step : {1, 5})
    {
        auto *button = new RepeatButton(QStringLiteral("+%1").arg(step), this);
        button->setFixedSize(40, 40);
        bpmRow->addWidget(button);
        connect(button, &QAbstractButton::pressed, this, [this, step]()
                { engine_->startBpmRepeat(step); });
        connect(button, &QAbstractButton::released, engine_, &MetronomeEngine::stopBpmRepeat);
    }
    column->addLayout(bpmRow);

    pendulum_ = new PendulumWidget(this);
    column->addWidget(pendulum_);

    beats_ = new BeatRowWidget(this);
    column->addWidget(beats_);
    auto *hint = new QLabel(txt("Pulsa un tiempo para cambiar su acento", "Click a beat to change its accent",
                                "Cliquez sur un temps pour changer son accent"),
                            this);
    hint->setObjectName(QStringLiteral("hint"));
    hint->setAlignment(Qt::AlignCenter);
    column->addWidget(hint);

    column->addSpacing(6);
    auto *tap = new QPushButton(QStringLiteral("TAP"), this);
    tap->setObjectName(QStringLiteral("tap"));
    tap->setFixedSize(112, 64);
    tap->setCursor(Qt::PointingHandCursor);
    tap->setFocusPolicy(Qt::NoFocus);
    tap->setToolTip(txt("Marca el tempo tocando (T)", "Tap the tempo (T)", "Marquez le tempo en tapant (T)"));
    playButton_ = new PlayButton(this);
    playButton_->setToolTip(txt("Tocar / pausar (Espacio)", "Play / pause (Space)", "Lecture / pause (Espace)"));
    auto *playRow = new QHBoxLayout;
    playRow->setSpacing(30);
    playRow->addStretch();
    playRow->addWidget(tap);
    playRow->addWidget(playButton_);
    playRow->addStretch();
    column->addLayout(playRow);
    column->addStretch();

    flash_.setDuration(180);
    flash_.setStartValue(0.22);
    flash_.setEndValue(0.0);
    flash_.setEasingCurve(QEasingCurve::OutQuad);
    connect(&flash_, &QVariantAnimation::valueChanged, this, [this](const QVariant &v)
            {
                flashAlpha_ = v.toReal();
                update();
            });

    connect(slider_, &QSlider::valueChanged, engine_, &MetronomeEngine::setBpm);
    connect(tap, &QPushButton::clicked, engine_, &MetronomeEngine::tap);
    connect(playButton_, &QAbstractButton::clicked, this, &TempoPanel::playClicked);
    connect(bpmLabel_, &ClickableLabel::clicked, this, &TempoPanel::askBpm);
    connect(beats_, &BeatRowWidget::tapped, this, [this](int index)
            {
                engine_->edit([index](const ClickConfig &c)
                              {
                                  ClickConfig next = c;
                                  if (index < next.accents.size())
                                      next.accents[index] = nextAccent(next.accents[index]);
                                  return next; });
            });
    connect(engine_, &MetronomeEngine::pulse, this, &TempoPanel::onPulse);
    connect(settings_, &AppSettings::changed, this, &TempoPanel::refresh);

    refresh();
}

bool TempoPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == banner_ && event->type() == QEvent::MouseButtonRelease)
        emit bannerClicked();
    return QWidget::eventFilter(watched, event);
}

void TempoPanel::setPlaying(bool playing)
{
    playButton_->setPlaying(playing);
}

void TempoPanel::setPracticeEnabled(bool enabled)
{
    practiceEnabled_ = enabled;
    updateBanner();
}

void TempoPanel::refresh()
{
    updateBpm();
    updateBanner();
    pendulum_->setVisible(settings_->pendulum());
    const LiveState &live = engine_->live();
    beats_->setAccents(engine_->config().accents);
    beats_->setCurrent(engine_->running() ? live.beat : -1, live.muted && engine_->running(), live.countIn && engine_->running());
}

void TempoPanel::updateBpm()
{
    const ClickConfig &config = engine_->config();
    const int shown = engine_->shownBpm();
    bpmLabel_->setText(QStringLiteral("%1<span style=\"font-size:24px\"> BPM</span>").arg(shown));
    tempoLabel_->setText(tempoNames(shown).join(QStringLiteral(" · ")));
    noteLabel_->setText(config.trainer.enabled
                            ? txt("meta %1 BPM", "target %1 BPM", "objectif %1 BPM").arg(config.trainer.targetBpm)
                            : txt("tempo en %1", "tempo at %1", "tempo à %1").arg(config.unit));
    if (slider_->value() != config.bpm)
    {
        const QSignalBlocker blocker(slider_);
        slider_->setValue(config.bpm);
    }
}

void TempoPanel::updateBanner()
{
    QString text;
    QString clockText;
    QString kind;
    if (const auto &run = engine_->setlistRun())
    {
        text = txt("Repertorio: %1 · %2/%3", "Setlist: %1 · %2/%3", "Répertoire : %1 · %2/%3")
                   .arg(run->name)
                   .arg(run->segment + 1)
                   .arg(run->names.size());
        kind = QStringLiteral("practice");
    }
    else if (practiceEnabled_)
    {
        switch (session_->phase())
        {
        case PracticeSession::Phase::Practice:
            text = txt("PRÁCTICA %1/%2", "PRACTICE %1/%2", "PRATIQUE %1/%2").arg(session_->round()).arg(session_->rounds());
            kind = QStringLiteral("practice");
            break;
        case PracticeSession::Phase::Rest:
            text = txt("DESCANSO", "BREAK", "PAUSE");
            kind = QStringLiteral("rest");
            break;
        case PracticeSession::Phase::Done:
            text = txt("SESIÓN TERMINADA", "SESSION COMPLETE", "SÉANCE TERMINÉE");
            kind = QStringLiteral("practice");
            break;
        default:
            text = txt("LISTO PARA PRACTICAR", "READY TO PRACTICE", "PRÊT À PRATIQUER");
            kind = QStringLiteral("practice");
        }
        clockText = clock(session_->remainingSeconds());
    }

    banner_->setVisible(!text.isEmpty());
    bannerText_->setText(text);
    bannerClock_->setText(clockText);
    bannerClock_->setVisible(!clockText.isEmpty());
    if (banner_->property("kind").toString() != kind)
    {
        banner_->setProperty("kind", kind);
        for (QWidget *w : {static_cast<QWidget *>(banner_), static_cast<QWidget *>(bannerText_), static_cast<QWidget *>(bannerClock_)})
        {
            w->style()->unpolish(w);
            w->style()->polish(w);
        }
    }
}

void TempoPanel::askBpm()
{
    bool ok = false;
    const int value = QInputDialog::getInt(this, txt("Escribir el tempo", "Type the tempo", "Saisir le tempo"), QStringLiteral("BPM"),
                                           engine_->config().bpm, ClickConfig::kMinBpm, ClickConfig::kMaxBpm, 1, &ok);
    if (ok)
        engine_->setBpm(value);
}

void TempoPanel::onPulse(const LiveState &live)
{
    const bool running = engine_->running();
    updateBpm();
    beats_->setCurrent(running ? live.beat : -1, live.muted && running, live.countIn && running);
    pendulum_->setRunning(running);
    if (running && live.beat >= 0)
    {
        pendulum_->swing(live.seq, 60000 / std::max(live.bpm, 1));
        if (settings_->flash() && !live.muted)
        {
            flash_.stop();
            flash_.start();
        }
    }
    // El reloj de la sesión avanza con su propio temporizador; esto solo refresca el aviso al cambiar de pieza.
    updateBanner();
}

void TempoPanel::paintEvent(QPaintEvent *)
{
    if (flashAlpha_ <= 0.0)
        return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor color(0x21, 0x96, 0xF3);
    color.setAlphaF(flashAlpha_);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(rect().adjusted(6, 6, -6, -6), 16, 16);
}
