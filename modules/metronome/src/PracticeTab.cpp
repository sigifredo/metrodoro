// own
#include <Controls.hpp>
#include <MetronomeEngine.hpp>
#include <PracticeSession.hpp>
#include <PracticeTab.hpp>
#include <StatsStore.hpp>
#include <Texts.hpp>

// qt
#include <QCheckBox>
#include <QHBoxLayout>
#include <QDate>
#include <QLabel>
#include <QLocale>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStyle>
#include <QVBoxLayout>

namespace
{
    QString minutes(int n)
    {
        return QStringLiteral("%1 min").arg(n);
    }

    QString barsText(int n)
    {
        return n == 1 ? txt("1 compás", "1 bar", "1 mesure") : txt("%1 compases", "%1 bars", "%1 mesures").arg(n);
    }

    QString profileName(int index)
    {
        switch (index)
        {
        case 0:
            return QStringLiteral("Sprint");
        case 1:
            return txt("Ligero", "Light", "Léger");
        case 2:
            return txt("Estándar", "Standard", "Standard");
        case 3:
            return txt("Intenso", "Intense", "Intense");
        default:
            return txt("Personalizado", "Custom", "Personnalisé");
        }
    }

    constexpr int kWeekDays = 7;
    constexpr int kMinScaleSeconds = 600;

    void repolish(QWidget *w)
    {
        w->style()->unpolish(w);
        w->style()->polish(w);
    }
}

/// Minutos con el metrónomo de los últimos siete días, en barras.
class WeekChart : public QWidget
{
public:
    explicit WeekChart(QWidget *parent) : QWidget(parent)
    {
        setMinimumHeight(110);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void setData(const QList<int> &seconds, const QStringList &labels)
    {
        seconds_ = seconds;
        labels_ = labels;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        if (seconds_.isEmpty())
            return;
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QFont small = font();
        small.setPixelSize(11);
        painter.setFont(small);

        int top = kMinScaleSeconds;
        for (int s : seconds_)
            top = std::max(top, s);
        const qreal cell = static_cast<qreal>(width()) / seconds_.size();
        const qreal barWidth = std::min<qreal>(cell * 0.5, 36);
        const qreal chartTop = 16;
        const qreal chartBottom = height() - 18;
        for (int i = 0; i < seconds_.size(); ++i)
        {
            const qreal x = i * cell + (cell - barWidth) / 2;
            const QRectF track(x, chartTop, barWidth, chartBottom - chartTop);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 255, 255, 25));
            painter.drawRoundedRect(track, 4, 4);
            const qreal h = track.height() * seconds_[i] / top;
            if (h > 0)
            {
                painter.setBrush(QColor(0x21, 0x96, 0xF3));
                painter.drawRoundedRect(QRectF(x, chartBottom - h, barWidth, h), 4, 4);
            }
            painter.setPen(QColor(0x88, 0x92, 0x99));
            painter.drawText(QRectF(i * cell, 0, cell, chartTop), Qt::AlignCenter, QString::number(seconds_[i] / 60));
            painter.drawText(QRectF(i * cell, chartBottom + 2, cell, 16), Qt::AlignCenter, labels_.value(i));
        }
    }

private:
    QList<int> seconds_;
    QStringList labels_;
};

PracticeTab::PracticeTab(MetronomeEngine *engine, PracticeSession *session, StatsStore *stats, QWidget *parent)
    : QWidget(parent), engine_(engine), session_(session), stats_(stats)
{
    auto *page = new QVBoxLayout(this);
    page->setContentsMargins(20, 20, 20, 20);
    page->setSpacing(16);
    QVBoxLayout *body = nullptr;

    // Modo práctica
    page->addWidget(makeCard(txt("Modo práctica", "Practice mode", "Mode pratique"), this, &body));
    practiceCheck_ = new QCheckBox(txt("Alternar práctica y descanso", "Alternate practice and breaks", "Alterner pratique et pause"), this);
    body->addWidget(makeSwitchRow(practiceCheck_,
                                  txt("El botón de tocar inicia la sesión: suena el metrónomo en la práctica y calla en el descanso",
                                      "The play button starts the session: the metronome sounds during practice and is silent during breaks",
                                      "Le bouton lecture lance la séance : le métronome joue pendant la pratique et se tait pendant la pause"),
                                  this));
    QStringList names;
    for (int i = 0; i <= PracticeSession::profiles().size(); ++i)
        names << profileName(i);
    profiles_ = new ChipRow(names, this);
    body->addWidget(profiles_);
    practiceMin_ = new StepperRow(txt("Práctica", "Practice", "Pratique"), this);
    restMin_ = new StepperRow(txt("Descanso", "Break", "Pause"), this);
    rounds_ = new StepperRow(txt("Rondas", "Rounds", "Tours"), this);
    body->addWidget(practiceMin_);
    body->addWidget(restMin_);
    body->addWidget(rounds_);
    total_ = new QLabel(this);
    total_->setObjectName(QStringLiteral("hint"));
    body->addWidget(total_);

    sessionBody_ = new QWidget(this);
    auto *sessionColumn = new QVBoxLayout(sessionBody_);
    sessionColumn->setContentsMargins(0, 4, 0, 0);
    sessionColumn->setSpacing(8);
    phase_ = new QLabel(sessionBody_);
    phase_->setObjectName(QStringLiteral("phase"));
    phase_->setAlignment(Qt::AlignCenter);
    countdown_ = new QLabel(sessionBody_);
    countdown_->setObjectName(QStringLiteral("countdown"));
    countdown_->setAlignment(Qt::AlignCenter);
    progress_ = new QProgressBar(sessionBody_);
    progress_->setObjectName(QStringLiteral("progress"));
    progress_->setRange(0, 1000);
    progress_->setTextVisible(false);
    progress_->setFixedHeight(5);
    toggle_ = new QPushButton(sessionBody_);
    toggle_->setObjectName(QStringLiteral("primary"));
    toggle_->setCursor(Qt::PointingHandCursor);
    stop_ = new QPushButton(txt("Detener", "Stop", "Arrêter"), sessionBody_);
    stop_->setObjectName(QStringLiteral("secondary"));
    stop_->setCursor(Qt::PointingHandCursor);
    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(toggle_);
    buttons->addWidget(stop_);
    buttons->addStretch();
    sessionColumn->addWidget(phase_);
    sessionColumn->addWidget(countdown_);
    sessionColumn->addWidget(progress_);
    sessionColumn->addLayout(buttons);
    body->addWidget(sessionBody_);

    // Entrenador de velocidad
    page->addWidget(makeCard(txt("Entrenador de velocidad", "Speed trainer", "Entraîneur de vitesse"), this, &body));
    trainerOn_ = new QCheckBox(txt("Subir el tempo poco a poco", "Raise the tempo step by step", "Monter le tempo petit à petit"), this);
    body->addWidget(makeSwitchRow(trainerOn_,
                                  txt("Empieza en el tempo actual y sube solo al terminar cada tramo de compases",
                                      "Starts at the current tempo and rises after each stretch of bars",
                                      "Part du tempo actuel et monte après chaque série de mesures"),
                                  this));
    trainerStep_ = new StepperRow(txt("Subir", "Raise by", "Monter de"), this);
    trainerEvery_ = new StepperRow(txt("Cada", "Every", "Toutes les"), this);
    trainerTarget_ = new StepperRow(txt("Meta", "Target", "Objectif"), this);
    trainerSummary_ = new QLabel(this);
    trainerSummary_->setObjectName(QStringLiteral("accent"));
    body->addWidget(trainerStep_);
    body->addWidget(trainerEvery_);
    body->addWidget(trainerTarget_);
    body->addWidget(trainerSummary_);

    // Compases mudos
    page->addWidget(makeCard(txt("Compases mudos", "Silent bars", "Mesures muettes"), this, &body));
    gapOn_ = new QCheckBox(txt("Silenciar compases", "Silence bars", "Rendre des mesures muettes"), this);
    body->addWidget(makeSwitchRow(gapOn_,
                                  txt("Para comprobar que mantienes el tempo tú solo: la interfaz sigue marcando los tiempos",
                                      "To check that you hold the tempo on your own: the display keeps showing the beats",
                                      "Pour vérifier que vous tenez le tempo seul : l'écran continue d'afficher les temps"),
                                  this));
    gapPlay_ = new StepperRow(txt("Suena", "Play", "Joue"), this);
    gapMute_ = new StepperRow(txt("Calla", "Mute", "Muet"), this);
    gapRandom_ = new QCheckBox(txt("Silencios al azar", "Random silences", "Silences aléatoires"), this);
    body->addWidget(gapPlay_);
    body->addWidget(gapMute_);
    body->addWidget(gapRandom_);

    // Constancia
    page->addWidget(makeCard(txt("Constancia", "Consistency", "Régularité"), this, &body));
    today_ = new QLabel(this);
    today_->setObjectName(QStringLiteral("strong"));
    streak_ = new QLabel(this);
    streak_->setObjectName(QStringLiteral("hint"));
    week_ = new WeekChart(this);
    body->addWidget(today_);
    body->addWidget(streak_);
    body->addWidget(week_);

    page->addStretch();

    connect(stats_, &StatsStore::changed, this, &PracticeTab::refreshStats);
    connect(practiceCheck_, &QCheckBox::toggled, this, &PracticeTab::practiceToggled);
    connect(profiles_, &ChipRow::activated, session_, &PracticeSession::setProfile);
    connect(practiceMin_, &StepperRow::stepped, session_, &PracticeSession::changePracticeMinutes);
    connect(restMin_, &StepperRow::stepped, session_, &PracticeSession::changeRestMinutes);
    connect(rounds_, &StepperRow::stepped, session_, &PracticeSession::changeRounds);
    connect(toggle_, &QPushButton::clicked, this, &PracticeTab::toggleRequested);
    connect(stop_, &QPushButton::clicked, this, &PracticeTab::stopRequested);

    const auto editTrainer = [this](auto change)
    { engine_->edit([change](const ClickConfig &c)
                    {
                        ClickConfig next = c;
                        change(next.trainer);
                        return next; }); };
    connect(trainerOn_, &QCheckBox::toggled, this, [editTrainer](bool on)
            { editTrainer([on](TrainerConfig &t)
                          { t.enabled = on; }); });
    connect(trainerStep_, &StepperRow::stepped, this, [editTrainer](int d)
            { editTrainer([d](TrainerConfig &t)
                          { t.stepBpm = std::clamp(t.stepBpm + d, 1, 20); }); });
    connect(trainerEvery_, &StepperRow::stepped, this, [editTrainer](int d)
            { editTrainer([d](TrainerConfig &t)
                          { t.everyBars = std::clamp(t.everyBars + d, 1, 32); }); });
    connect(trainerTarget_, &StepperRow::stepped, this, [editTrainer](int d)
            { editTrainer([d](TrainerConfig &t)
                          { t.targetBpm = std::clamp(t.targetBpm + d * 5, ClickConfig::kMinBpm, ClickConfig::kMaxBpm); }); });

    const auto editGap = [this](auto change)
    { engine_->edit([change](const ClickConfig &c)
                    {
                        ClickConfig next = c;
                        change(next.gap);
                        return next; }); };
    connect(gapOn_, &QCheckBox::toggled, this, [editGap](bool on)
            { editGap([on](GapConfig &g)
                      { g.enabled = on; }); });
    connect(gapPlay_, &StepperRow::stepped, this, [editGap](int d)
            { editGap([d](GapConfig &g)
                      { g.playBars = std::clamp(g.playBars + d, 1, 16); }); });
    connect(gapMute_, &StepperRow::stepped, this, [editGap](int d)
            { editGap([d](GapConfig &g)
                      { g.muteBars = std::clamp(g.muteBars + d, 1, 16); }); });
    connect(gapRandom_, &QCheckBox::toggled, this, [editGap](bool on)
            { editGap([on](GapConfig &g)
                      { g.random = on; }); });

    refreshSession();
    refreshConfig();
    refreshStats();
}

void PracticeTab::refreshStats()
{
    const QDate today = QDate::currentDate();
    QList<int> seconds;
    QStringList labels;
    for (int i = kWeekDays - 1; i >= 0; --i)
    {
        const QDate day = today.addDays(-i);
        seconds << stats_->secondsOn(day);
        labels << QLocale().dayName(day.dayOfWeek(), QLocale::NarrowFormat).toUpper();
    }
    today_->setText(txt("Hoy: %1 min con el metrónomo", "Today: %1 min with the metronome", "Aujourd'hui : %1 min avec le métronome")
                        .arg(stats_->secondsOn(today) / 60));
    const int streak = streakDays(stats_->days(), today);
    streak_->setText(streak == 1 ? txt("Racha: 1 día", "Streak: 1 day", "Série : 1 jour")
                                 : txt("Racha: %1 días", "Streak: %1 days", "Série : %1 jours").arg(streak));
    week_->setData(seconds, labels);
}

bool PracticeTab::practiceEnabled() const
{
    return practiceCheck_->isChecked();
}

void PracticeTab::setPracticeEnabled(bool enabled)
{
    const QSignalBlocker blocker(practiceCheck_);
    practiceCheck_->setChecked(enabled);
    refreshSession();
}

void PracticeTab::refreshSession()
{
    const bool enabled = practiceCheck_->isChecked();
    for (QWidget *w : {static_cast<QWidget *>(profiles_), static_cast<QWidget *>(practiceMin_), static_cast<QWidget *>(restMin_),
                       static_cast<QWidget *>(rounds_), static_cast<QWidget *>(total_)})
        w->setEnabled(enabled);
    sessionBody_->setVisible(enabled);

    practiceMin_->setText(minutes(session_->practiceMinutes()));
    restMin_->setText(minutes(session_->restMinutes()));
    rounds_->setText(QString::number(session_->rounds()));
    total_->setText(txt("Total: %1 min", "Total: %1 min", "Total : %1 min").arg(session_->totalMinutes()));
    profiles_->setCurrent(session_->matchingProfile());

    const auto phase = session_->phase();
    const bool active = phase == PracticeSession::Phase::Practice || phase == PracticeSession::Phase::Rest;
    QString label;
    switch (phase)
    {
    case PracticeSession::Phase::Practice:
        label = txt("PRÁCTICA %1/%2", "PRACTICE %1/%2", "PRATIQUE %1/%2").arg(session_->round()).arg(session_->rounds());
        break;
    case PracticeSession::Phase::Rest:
        label = txt("DESCANSO", "BREAK", "PAUSE");
        break;
    case PracticeSession::Phase::Done:
        label = txt("SESIÓN TERMINADA", "SESSION COMPLETE", "SÉANCE TERMINÉE");
        break;
    default:
        label = txt("LISTO PARA PRACTICAR", "READY TO PRACTICE", "PRÊT À PRATIQUER");
    }
    phase_->setText(label);
    phase_->setProperty("rest", phase == PracticeSession::Phase::Rest);
    countdown_->setProperty("rest", phase == PracticeSession::Phase::Rest);
    progress_->setProperty("rest", phase == PracticeSession::Phase::Rest);
    for (QWidget *w : {static_cast<QWidget *>(phase_), static_cast<QWidget *>(countdown_), static_cast<QWidget *>(progress_)})
        repolish(w);

    const int remaining = session_->remainingSeconds();
    countdown_->setText(QStringLiteral("%1:%2").arg(remaining / 60, 2, 10, QLatin1Char('0')).arg(remaining % 60, 2, 10, QLatin1Char('0')));
    const int total = session_->phaseSeconds();
    progress_->setVisible(active && total > 0);
    progress_->setValue(active && total > 0 ? 1000 - remaining * 1000 / total : 0);

    toggle_->setText(session_->running() ? txt("Pausar", "Pause", "Pause")
                                         : (active ? txt("Reanudar", "Resume", "Reprendre") : txt("Empezar", "Start", "Démarrer")));
    stop_->setVisible(active || phase == PracticeSession::Phase::Done);
}

void PracticeTab::refreshConfig()
{
    const ClickConfig &c = engine_->config();
    const auto sync = [](QCheckBox *box, bool value)
    {
        const QSignalBlocker blocker(box);
        box->setChecked(value);
    };

    const TrainerConfig &t = c.trainer;
    sync(trainerOn_, t.enabled);
    trainerStep_->setText(QStringLiteral("+%1").arg(t.stepBpm));
    trainerEvery_->setText(barsText(t.everyBars));
    trainerTarget_->setText(QString::number(t.targetBpm));
    const int steps = t.targetBpm == c.bpm ? 0 : (std::abs(t.targetBpm - c.bpm) + t.stepBpm - 1) / t.stepBpm;
    trainerSummary_->setText(txt("De %1 a %2 BPM en %3 pasos", "From %1 to %2 BPM in %3 steps", "De %1 à %2 BPM en %3 étapes")
                                 .arg(c.bpm)
                                 .arg(t.targetBpm)
                                 .arg(steps));
    trainerSummary_->setVisible(t.enabled);

    const GapConfig &g = c.gap;
    sync(gapOn_, g.enabled);
    gapPlay_->setText(barsText(g.playBars));
    gapMute_->setText(barsText(g.muteBars));
    sync(gapRandom_, g.random);
}
