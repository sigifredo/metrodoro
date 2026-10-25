// own
#include <BeatDotsWidget.hpp>
#include <MainWindow.hpp>
#include <MetronomeEngine.hpp>
#include <NoteSelectorWidget.hpp>
#include <PlayButton.hpp>
#include <PracticeSession.hpp>
#include <RepeatButton.hpp>

// qt
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QSlider>
#include <QStyle>
#include <QVBoxLayout>

// std
#include <functional>

namespace
{
    struct Strings
    {
        QString stressFirstBeat;
        QString practiceMode;
        QString practice;
        QString rest;
        QString rounds;
        QString total;
        QString minutes;
        QString ready;
        QString done;
        QString custom;
        QString profiles[4];
    };

    Strings stringsForLocale()
    {
        switch (QLocale().language())
        {
        case QLocale::Spanish:
            return {QStringLiteral("MARCAR EL PRIMER TIEMPO"), QStringLiteral("MODO PRÁCTICA"),
                    QStringLiteral("PRÁCTICA"), QStringLiteral("DESCANSO"), QStringLiteral("RONDAS"),
                    QStringLiteral("TOTAL"), QStringLiteral("min"), QStringLiteral("LISTO PARA PRACTICAR"),
                    QStringLiteral("SESIÓN TERMINADA"), QStringLiteral("Personalizado"),
                    {QStringLiteral("Sprint"), QStringLiteral("Ligero"), QStringLiteral("Estándar"),
                     QStringLiteral("Intenso")}};
        case QLocale::English:
            return {QStringLiteral("STRESS FIRST BEAT"), QStringLiteral("PRACTICE MODE"),
                    QStringLiteral("PRACTICE"), QStringLiteral("BREAK"), QStringLiteral("ROUNDS"),
                    QStringLiteral("TOTAL"), QStringLiteral("min"), QStringLiteral("READY TO PRACTICE"),
                    QStringLiteral("SESSION COMPLETE"), QStringLiteral("Custom"),
                    {QStringLiteral("Sprint"), QStringLiteral("Light"), QStringLiteral("Standard"),
                     QStringLiteral("Intense")}};
        default:
            return {QStringLiteral("ACCENTUER LE PREMIER TEMPS"), QStringLiteral("MODE PRATIQUE"),
                    QStringLiteral("PRATIQUE"), QStringLiteral("PAUSE"), QStringLiteral("TOURS"),
                    QStringLiteral("TOTAL"), QStringLiteral("min"), QStringLiteral("PRÊT À PRATIQUER"),
                    QStringLiteral("SÉANCE TERMINÉE"), QStringLiteral("Personnalisé"),
                    {QStringLiteral("Sprint"), QStringLiteral("Léger"), QStringLiteral("Standard"),
                     QStringLiteral("Intense")}};
        }
    }

    // Etiqueta "−  valor  +" para una fila de ajuste.
    QLabel *addStepper(QVBoxLayout *column, QWidget *parent, const QString &title,
                       const std::function<void(int)> &onStep)
    {
        auto *titleLabel = new QLabel(title, parent);
        auto *value = new QLabel(parent);
        value->setObjectName(QStringLiteral("counter"));
        value->setAlignment(Qt::AlignCenter);
        value->setFixedWidth(80);
        auto *minus = new RepeatButton(QStringLiteral("−"), parent);
        auto *plus = new RepeatButton(QStringLiteral("+"), parent);
        minus->setFixedSize(30, 30);
        plus->setFixedSize(30, 30);
        auto *row = new QHBoxLayout;
        row->addWidget(titleLabel);
        row->addStretch();
        row->addWidget(minus);
        row->addWidget(value);
        row->addWidget(plus);
        column->addLayout(row);
        QObject::connect(minus, &QAbstractButton::clicked, parent, [onStep]()
                         { onStep(-1); });
        QObject::connect(plus, &QAbstractButton::clicked, parent, [onStep]()
                         { onStep(+1); });
        return value;
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
    : QWidget(parent), engine_(new MetronomeEngine(this)), session_(new PracticeSession(engine_, this))
{
    setWindowTitle(QStringLiteral("Metronome"));
    buildUi();

    connect(engine_, &MetronomeEngine::bpmChanged, this, &MainWindow::updateBpm);
    connect(engine_, &MetronomeEngine::beatChanged, dots_, &BeatDotsWidget::setCurrent);
    connect(engine_, &MetronomeEngine::playingChanged, this, [this](bool playing)
            {
                if (!practiceCheck_->isChecked())
                    playButton_->setPlaying(playing);
            });
    connect(session_, &PracticeSession::changed, this, &MainWindow::updatePractice);
    connect(session_, &PracticeSession::runningChanged, this, [this](bool running)
            {
                if (practiceCheck_->isChecked())
                    playButton_->setPlaying(running);
            });
    connect(session_, &PracticeSession::transitioned, this, []()
            { QApplication::beep(); });
    connect(engine_, &MetronomeEngine::timeInMeasureChanged, this, [this](int count)
            {
                measureLabel_->setText(QString::number(count));
                dots_->setBeats(count);
            });
    connect(engine_, &MetronomeEngine::subdivisionChanged, this, [this]()
            { notes_->setSelection(engine_->subdivision(), engine_->shuffle()); });

    updateBpm();
    updatePractice();
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
    connect(playButton_, &QAbstractButton::clicked, this, [this]()
            {
                if (practiceCheck_->isChecked())
                    session_->toggle();
                else
                    engine_->playPause();
            });

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

    // Sesión de práctica (pomodoro)
    column->addWidget(makeSeparator(content));
    practiceCheck_ = new QCheckBox(strings.practiceMode, content);
    profileCombo_ = new QComboBox(content);
    for (int i = 0; i < PracticeSession::profiles().size(); ++i)
        profileCombo_->addItem(strings.profiles[i]);
    profileCombo_->addItem(strings.custom);
    auto *modeRow = new QHBoxLayout;
    modeRow->addWidget(practiceCheck_);
    modeRow->addStretch();
    modeRow->addWidget(profileCombo_);
    column->addLayout(modeRow);

    // Todo lo que depende de que el modo práctica esté activo vive aquí para atenuarlo junto.
    practiceBody_ = new QWidget(content);
    auto *body = new QVBoxLayout(practiceBody_);
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(12);
    column->addWidget(practiceBody_);

    phaseLabel_ = new QLabel(practiceBody_);
    phaseLabel_->setObjectName(QStringLiteral("phase"));
    phaseLabel_->setAlignment(Qt::AlignCenter);
    countdownLabel_ = new QLabel(practiceBody_);
    countdownLabel_->setObjectName(QStringLiteral("countdown"));
    countdownLabel_->setAlignment(Qt::AlignCenter);
    body->addWidget(phaseLabel_);
    body->addWidget(countdownLabel_);

    practiceMinLabel_ = addStepper(body, practiceBody_, strings.practice, [this](int d)
                                   { session_->changePracticeMinutes(d); });
    restMinLabel_ = addStepper(body, practiceBody_, strings.rest, [this](int d)
                               { session_->changeRestMinutes(d); });
    roundsLabel_ = addStepper(body, practiceBody_, strings.rounds, [this](int d)
                              { session_->changeRounds(d); });
    totalLabel_ = new QLabel(practiceBody_);
    totalLabel_->setAlignment(Qt::AlignCenter);
    body->addWidget(totalLabel_);

    connect(practiceCheck_, &QCheckBox::toggled, this, [this]()
            {
                session_->stop();
                playButton_->setPlaying(false);
                updatePractice();
            });
    connect(profileCombo_, &QComboBox::activated, this, [this](int index)
            { session_->setProfile(index); });
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

void MainWindow::updatePractice()
{
    const Strings strings = stringsForLocale();
    const bool enabled = practiceCheck_->isChecked();

    profileCombo_->setEnabled(enabled);
    practiceBody_->setEnabled(enabled);

    practiceMinLabel_->setText(QStringLiteral("%1 %2").arg(session_->practiceMinutes()).arg(strings.minutes));
    restMinLabel_->setText(QStringLiteral("%1 %2").arg(session_->restMinutes()).arg(strings.minutes));
    roundsLabel_->setText(QString::number(session_->rounds()));
    totalLabel_->setText(QStringLiteral("%1: %2 %3").arg(strings.total).arg(session_->totalMinutes()).arg(strings.minutes));

    const QSignalBlocker blocker(profileCombo_);
    profileCombo_->setCurrentIndex(session_->matchingProfile());

    QString phase;
    switch (session_->phase())
    {
    case PracticeSession::Phase::Practice:
        phase = QStringLiteral("%1 %2/%3").arg(strings.practice).arg(session_->round()).arg(session_->rounds());
        break;
    case PracticeSession::Phase::Rest:
        phase = strings.rest;
        break;
    case PracticeSession::Phase::Done:
        phase = strings.done;
        break;
    default:
        phase = strings.ready;
    }
    phaseLabel_->setText(phase);
    phaseLabel_->setProperty("rest", session_->phase() == PracticeSession::Phase::Rest);
    phaseLabel_->style()->unpolish(phaseLabel_);
    phaseLabel_->style()->polish(phaseLabel_);

    const int remaining = session_->remainingSeconds();
    countdownLabel_->setText(QStringLiteral("%1:%2")
                                 .arg(remaining / 60, 2, 10, QLatin1Char('0'))
                                 .arg(remaining % 60, 2, 10, QLatin1Char('0')));
}
