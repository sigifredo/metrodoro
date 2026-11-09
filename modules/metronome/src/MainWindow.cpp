// own
#include <Controls.hpp>
#include <LibraryStore.hpp>
#include <LibraryTab.hpp>
#include <MainWindow.hpp>
#include <MetronomeEngine.hpp>
#include <MetronomeTab.hpp>
#include <PhaseChime.hpp>
#include <PracticeSession.hpp>
#include <PracticeTab.hpp>
#include <SettingsTab.hpp>
#include <StatsStore.hpp>
#include <TempoPanel.hpp>
#include <Texts.hpp>

// qt
#include <QFrame>
#include <QHBoxLayout>
#include <QShortcut>
#include <QTabWidget>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), settings_(new AppSettings(this)), engine_(new MetronomeEngine(this)),
      session_(new PracticeSession(engine_, this)), stats_(new StatsStore(this)), library_(new LibraryStore(this)), chime_(new PhaseChime(this))
{
    setWindowTitle(QStringLiteral("Metrodoro"));
    setMinimumSize(900, 660);
    resize(1080, 740);

    stats_->track(engine_);

    // La aplicación arranca donde se dejó.
    engine_->setConfig(settings_->loadConfig());
    engine_->setVolume(settings_->volume());
    savedPractice_ = settings_->loadPractice();
    practiceEnabled_ = savedPractice_.enabled;
    session_->configure(savedPractice_.practiceMin, savedPractice_.restMin, savedPractice_.rounds);

    buildUi();
    buildShortcuts();

    connect(engine_, &MetronomeEngine::configChanged, this, &MainWindow::onConfigChanged);
    connect(engine_, &MetronomeEngine::runningChanged, this, &MainWindow::updatePlayButton);
    connect(engine_, &MetronomeEngine::setlistChanged, this, [this]()
            {
                tempo_->refresh();
                updatePlayButton();
            });
    connect(session_, &PracticeSession::changed, this, &MainWindow::onSessionChanged);
    connect(session_, &PracticeSession::runningChanged, this, &MainWindow::updatePlayButton);
    connect(session_, &PracticeSession::transitioned, this, [this](PracticeSession::Transition t)
            {
                if (settings_->phaseSound())
                    chime_->play(t != PracticeSession::Transition::ToRest, t == PracticeSession::Transition::Finished ? 3 : 2, settings_->volume()); });
    connect(settings_, &AppSettings::changed, this, [this]()
            {
                engine_->setVolume(settings_->volume());
                metronomeTab_->refresh();
                settingsTab_->refresh();
            });

    onConfigChanged();
    onSessionChanged();
}

void MainWindow::buildUi()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    tempo_ = new TempoPanel(engine_, session_, settings_, this);
    tempo_->setFixedWidth(500);
    tempo_->setPracticeEnabled(practiceEnabled_);
    root->addWidget(tempo_);

    auto *divider = new QFrame(this);
    divider->setObjectName(QStringLiteral("divider"));
    divider->setFrameShape(QFrame::VLine);
    root->addWidget(divider);

    tabs_ = new QTabWidget(this);
    tabs_->setDocumentMode(true);
    tabs_->setUsesScrollButtons(false);
    metronomeTab_ = new MetronomeTab(engine_, settings_, this);
    practiceTab_ = new PracticeTab(engine_, session_, stats_, this);
    libraryTab_ = new LibraryTab(engine_, session_, library_, this);
    settingsTab_ = new SettingsTab(settings_, stats_, this);
    practiceTab_->setPracticeEnabled(practiceEnabled_);
    tabs_->addTab(makeScrollPage(metronomeTab_, tabs_), txt("Metrónomo", "Metronome", "Métronome"));
    tabs_->addTab(makeScrollPage(practiceTab_, tabs_), txt("Práctica", "Practice", "Pratique"));
    tabs_->addTab(makeScrollPage(libraryTab_, tabs_), txt("Biblioteca", "Library", "Bibliothèque"));
    tabs_->addTab(makeScrollPage(settingsTab_, tabs_), txt("Ajustes", "Settings", "Réglages"));
    root->addWidget(tabs_, 1);

    connect(tempo_, &TempoPanel::playClicked, this, &MainWindow::togglePlayback);
    connect(tempo_, &TempoPanel::bannerClicked, this, [this]()
            { tabs_->setCurrentIndex(1); });
    connect(practiceTab_, &PracticeTab::practiceToggled, this, [this](bool enabled)
            {
                stopAll();
                practiceEnabled_ = enabled;
                tempo_->setPracticeEnabled(enabled);
                practiceTab_->refreshSession();
                persistPractice();
            });
    connect(practiceTab_, &PracticeTab::toggleRequested, this, &MainWindow::togglePlayback);
    connect(practiceTab_, &PracticeTab::stopRequested, this, &MainWindow::stopAll);
}

void MainWindow::buildShortcuts()
{
    const auto add = [this](const QKeySequence &keys, auto action)
    {
        auto *shortcut = new QShortcut(keys, this);
        connect(shortcut, &QShortcut::activated, this, action);
    };
    add(Qt::Key_Space, [this]()
        { togglePlayback(); });
    add(Qt::CTRL | Qt::Key_S, [this]()
        {
            tabs_->setCurrentWidget(tabs_->widget(2));
            libraryTab_->saveCurrentAsPreset(); });
    add(Qt::Key_T, [this]()
        { engine_->tap(); });
    add(Qt::CTRL | Qt::Key_Up, [this]()
        { engine_->setBpm(engine_->config().bpm + 1); });
    add(Qt::CTRL | Qt::Key_Down, [this]()
        { engine_->setBpm(engine_->config().bpm - 1); });
    add(Qt::CTRL | Qt::SHIFT | Qt::Key_Up, [this]()
        { engine_->setBpm(engine_->config().bpm + 5); });
    add(Qt::CTRL | Qt::SHIFT | Qt::Key_Down, [this]()
        { engine_->setBpm(engine_->config().bpm - 5); });
    for (int i = 0; i < tabs_->count(); ++i)
    {
        add(Qt::CTRL | static_cast<Qt::Key>(Qt::Key_1 + i), [this, i]()
            { tabs_->setCurrentIndex(i); });
    }
}

void MainWindow::onConfigChanged()
{
    tempo_->refresh();
    metronomeTab_->refresh();
    practiceTab_->refreshConfig();
    // Durante un repertorio la configuración la manda la pieza en curso y no se recuerda.
    if (!engine_->setlistRun())
        settings_->saveConfig(engine_->config());
}

void MainWindow::onSessionChanged()
{
    practiceTab_->refreshSession();
    tempo_->refresh();
    persistPractice();
}

void MainWindow::persistPractice()
{
    const PracticePrefs now{practiceEnabled_, session_->practiceMinutes(), session_->restMinutes(), session_->rounds()};
    if (now.enabled == savedPractice_.enabled && now.practiceMin == savedPractice_.practiceMin &&
        now.restMin == savedPractice_.restMin && now.rounds == savedPractice_.rounds)
        return;
    savedPractice_ = now;
    settings_->savePractice(now);
}

void MainWindow::updatePlayButton()
{
    // El botón sigue al motor con el modo práctica apagado y a la sesión con él encendido: durante un
    // descanso el motor calla pero la sesión corre, y el botón debe seguir encendido.
    tempo_->setPlaying(engine_->running() || session_->running());
    tempo_->refresh();
}

void MainWindow::togglePlayback()
{
    if (practiceEnabled_ && !engine_->setlistRun())
        session_->toggle();
    else
        engine_->toggle();
}

void MainWindow::stopAll()
{
    session_->stop();
    engine_->exitSetlist();
    engine_->stop();
    updatePlayButton();
}
