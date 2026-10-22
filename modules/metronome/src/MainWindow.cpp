// own
#include <BeatDotsWidget.hpp>
#include <MainWindow.hpp>
#include <MetronomeEngine.hpp>
#include <NoteSelectorWidget.hpp>
#include <PlayButton.hpp>
#include <RepeatButton.hpp>

// qt
#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

namespace
{
    struct Strings
    {
        QString stressFirstBeat;
        QString timer;
    };

    Strings stringsForLocale()
    {
        switch (QLocale().language())
        {
        case QLocale::Spanish:
            return {QStringLiteral("MARCAR EL PRIMER TIEMPO"), QStringLiteral("TEMPORIZADOR")};
        case QLocale::English:
            return {QStringLiteral("STRESS FIRST BEAT"), QStringLiteral("TIMER")};
        default:
            return {QStringLiteral("ACCENTUER LE PREMIER TEMPS"), QStringLiteral("CHRONOMETRE")};
        }
    }

    QFrame *makeSeparator(QWidget *parent)
    {
        auto *line = new QFrame(parent);
        line->setObjectName(QStringLiteral("separator"));
        line->setFrameShape(QFrame::HLine);
        return line;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), engine_(new MetronomeEngine(this))
{
    setWindowTitle(QStringLiteral("Metronome"));
    buildUi();

    connect(engine_, &MetronomeEngine::bpmChanged, this, &MainWindow::updateBpm);
    connect(engine_, &MetronomeEngine::beatChanged, dots_, &BeatDotsWidget::setCurrent);
    connect(engine_, &MetronomeEngine::playingChanged, playButton_, &PlayButton::setPlaying);
    connect(engine_, &MetronomeEngine::timerChanged, this, &MainWindow::updateTimer);
    connect(engine_, &MetronomeEngine::timeInMeasureChanged, this, [this](int count)
            {
                measureLabel_->setText(QString::number(count));
                dots_->setBeats(count);
            });
    connect(engine_, &MetronomeEngine::subdivisionChanged, this, [this]()
            { notes_->setSelection(engine_->subdivision(), engine_->shuffle()); });

    updateBpm();
    updateTimer();
}

void MainWindow::buildUi()
{
    const Strings strings = stringsForLocale();

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->addStretch();

    auto *content = new QWidget(this);
    content->setMaximumWidth(520);
    auto *column = new QVBoxLayout(content);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(12);
    root->addWidget(content, 0, Qt::AlignHCenter);
    root->addStretch();

    // BPM + nombre del tempo
    bpmLabel_ = new QLabel(content);
    bpmLabel_->setObjectName(QStringLiteral("bpm"));
    bpmLabel_->setAlignment(Qt::AlignCenter);
    bpmLabel_->setTextFormat(Qt::RichText);
    tempoLabel_ = new QLabel(content);
    tempoLabel_->setObjectName(QStringLiteral("tempo"));
    tempoLabel_->setAlignment(Qt::AlignCenter);
    column->addWidget(bpmLabel_);
    column->addWidget(tempoLabel_);

    // - slider +
    auto *minus = new RepeatButton(QStringLiteral("−"), content);
    auto *plus = new RepeatButton(QStringLiteral("+"), content);
    slider_ = new QSlider(Qt::Horizontal, content);
    slider_->setRange(MetronomeEngine::kSliderMin, MetronomeEngine::kSliderMax);
    auto *bpmRow = new QHBoxLayout;
    bpmRow->setSpacing(20);
    bpmRow->addWidget(minus);
    bpmRow->addWidget(slider_, 1);
    bpmRow->addWidget(plus);
    column->addLayout(bpmRow);

    connect(slider_, &QSlider::valueChanged, engine_, &MetronomeEngine::setBpm);
    connect(minus, &QAbstractButton::pressed, this, [this]()
            { engine_->startBpmRepeat(-1); });
    connect(plus, &QAbstractButton::pressed, this, [this]()
            { engine_->startBpmRepeat(+1); });
    connect(minus, &QAbstractButton::released, engine_, &MetronomeEngine::stopBpmRepeat);
    connect(plus, &QAbstractButton::released, engine_, &MetronomeEngine::stopBpmRepeat);

    // Tiempos del compás
    dots_ = new BeatDotsWidget(content);
    dots_->setBeats(engine_->timeInMeasure());
    column->addWidget(dots_);

    // TAP + play
    auto *tap = new QPushButton(QStringLiteral("TAP"), content);
    tap->setObjectName(QStringLiteral("tap"));
    tap->setFixedSize(112, 64);
    tap->setCursor(Qt::PointingHandCursor);
    tap->setFocusPolicy(Qt::NoFocus);
    playButton_ = new PlayButton(content);
    auto *playRow = new QHBoxLayout;
    playRow->setSpacing(30);
    playRow->addStretch();
    playRow->addWidget(tap);
    playRow->addWidget(playButton_);
    playRow->addStretch();
    column->addLayout(playRow);

    connect(tap, &QPushButton::clicked, engine_, &MetronomeEngine::tap);
    connect(playButton_, &QAbstractButton::clicked, engine_, &MetronomeEngine::playPause);

    // Acentuar primer tiempo + tiempos por compás
    accentCheck_ = new QCheckBox(strings.stressFirstBeat, content);
    measureLabel_ = new QLabel(QString::number(engine_->timeInMeasure()), content);
    measureLabel_->setObjectName(QStringLiteral("counter"));
    measureLabel_->setAlignment(Qt::AlignCenter);
    measureLabel_->setFixedWidth(60);
    auto *measureMinus = new RepeatButton(QStringLiteral("−"), content);
    auto *measurePlus = new RepeatButton(QStringLiteral("+"), content);
    measureMinus->setFixedSize(30, 30);
    measurePlus->setFixedSize(30, 30);
    auto *measureRow = new QHBoxLayout;
    measureRow->addWidget(accentCheck_);
    measureRow->addStretch();
    measureRow->addWidget(measureMinus);
    measureRow->addWidget(measureLabel_);
    measureRow->addWidget(measurePlus);
    column->addSpacing(8);
    column->addLayout(measureRow);

    connect(accentCheck_, &QCheckBox::toggled, engine_, &MetronomeEngine::setAccentFirstBeat);
    connect(measureMinus, &QAbstractButton::clicked, this, [this]()
            { engine_->changeTimeInMeasure(-1); });
    connect(measurePlus, &QAbstractButton::clicked, this, [this]()
            { engine_->changeTimeInMeasure(+1); });

    // Subdivisiones
    notes_ = new NoteSelectorWidget(content);
    column->addSpacing(8);
    column->addWidget(notes_);
    connect(notes_, &NoteSelectorWidget::selected, engine_, &MetronomeEngine::setSubdivision);

    // Temporizador
    column->addWidget(makeSeparator(content));
    timerCheck_ = new QCheckBox(strings.timer, content);
    timerLabel_ = new QLabel(content);
    timerLabel_->setObjectName(QStringLiteral("counter"));
    timerLabel_->setAlignment(Qt::AlignCenter);
    timerLabel_->setFixedWidth(60);
    auto *timerMinus = new RepeatButton(QStringLiteral("−"), content);
    auto *timerPlus = new RepeatButton(QStringLiteral("+"), content);
    timerMinus->setFixedSize(30, 30);
    timerPlus->setFixedSize(30, 30);
    auto *timerRow = new QHBoxLayout;
    timerRow->addWidget(timerCheck_);
    timerRow->addStretch();
    timerRow->addWidget(timerMinus);
    timerRow->addWidget(timerLabel_);
    timerRow->addWidget(timerPlus);
    column->addLayout(timerRow);

    connect(timerCheck_, &QCheckBox::clicked, engine_, &MetronomeEngine::setTimerActive);
    connect(timerMinus, &QAbstractButton::clicked, this, [this]()
            { engine_->changeCountdown(-60); });
    connect(timerPlus, &QAbstractButton::clicked, this, [this]()
            { engine_->changeCountdown(+60); });
}

void MainWindow::updateBpm()
{
    const int bpm = engine_->bpm();
    bpmLabel_->setText(QStringLiteral("%1<span style=\"font-size:24px\"> BPM</span>").arg(bpm));
    tempoLabel_->setText(MetronomeEngine::tempoLabels(bpm).join(QStringLiteral(" - ")));

    if (slider_->value() != bpm)
    {
        const QSignalBlocker blocker(slider_);
        slider_->setValue(bpm);
    }
}

void MainWindow::updateTimer()
{
    const int remaining = engine_->timerRemaining();
    timerLabel_->setText(QStringLiteral("%1:%2")
                             .arg(remaining / 60, 2, 10, QLatin1Char('0'))
                             .arg(remaining % 60, 2, 10, QLatin1Char('0')));

    if (timerCheck_->isChecked() != engine_->timerActive())
    {
        const QSignalBlocker blocker(timerCheck_);
        timerCheck_->setChecked(engine_->timerActive());
    }
}
