// own
#include <MetronomeEngine.hpp>
#include <PracticeSession.hpp>

// std
#include <algorithm>

namespace
{
    constexpr int kTickMs = 250;
}

PracticeSession::PracticeSession(MetronomeEngine *engine, QObject *parent)
    : QObject(parent), engine_(engine)
{
    clock_.start();
    tick_.setInterval(kTickMs);
    connect(&tick_, &QTimer::timeout, this, &PracticeSession::onTick);
}

const QList<PracticeSession::Profile> &PracticeSession::profiles()
{
    // Bloques cortos y enfocados, con descanso real entre ellos.
    static const QList<Profile> list = {
        {"sprint", 5, 1, 6},    // pasajes difíciles / velocidad: 35 min
        {"light", 10, 2, 2},    // calentamiento, días de poca energía: 22 min
        {"standard", 15, 3, 3}, // sesión diaria: 51 min
        {"intense", 25, 5, 3},  // sesión larga tipo pomodoro: 85 min
    };
    return list;
}

int PracticeSession::remainingSeconds() const
{
    switch (phase_)
    {
    case Phase::Practice:
    case Phase::Rest:
    {
        const qint64 leftMs = running_ ? phaseEndMs_ - clock_.elapsed() : pausedLeftMs_;
        return static_cast<int>((std::max<qint64>(leftMs, 0) + 999) / 1000);
    }
    default:
        return practiceMin_ * 60;
    }
}

int PracticeSession::totalMinutes() const
{
    return practiceMin_ * rounds_ + restMin_ * (rounds_ - 1);
}

int PracticeSession::matchingProfile() const
{
    const auto &list = profiles();
    for (int i = 0; i < list.size(); ++i)
    {
        if (list[i].practiceMin == practiceMin_ && list[i].restMin == restMin_ && list[i].rounds == rounds_)
            return i;
    }
    return list.size();
}

/* ---- Control ------------------------------------------------------------- */

void PracticeSession::toggle()
{
    if (phase_ == Phase::Idle || phase_ == Phase::Done)
        start();
    else if (running_)
        pause();
    else
        resume();
}

void PracticeSession::stop()
{
    tick_.stop();
    engine_->stop();
    phase_ = Phase::Idle;
    round_ = 1;
    setRunning(false);
    emit changed();
}

void PracticeSession::start()
{
    round_ = 1;
    enterPractice();
}

void PracticeSession::pause()
{
    pausedLeftMs_ = phaseEndMs_ - clock_.elapsed();
    tick_.stop();
    if (phase_ == Phase::Practice)
        engine_->playPause();
    setRunning(false);
    emit changed();
}

void PracticeSession::resume()
{
    phaseEndMs_ = clock_.elapsed() + pausedLeftMs_;
    tick_.start();
    if (phase_ == Phase::Practice)
        engine_->playPause();
    setRunning(true);
    emit changed();
}

/* ---- Fases --------------------------------------------------------------- */

void PracticeSession::beginPhase(Phase phase, int seconds)
{
    phase_ = phase;
    phaseEndMs_ = clock_.elapsed() + static_cast<qint64>(seconds) * 1000;
    tick_.start();
    setRunning(true);
    emit changed();
}

void PracticeSession::enterPractice()
{
    engine_->stop();
    engine_->playPause();
    beginPhase(Phase::Practice, practiceMin_ * 60);
}

void PracticeSession::enterRest()
{
    engine_->stop();
    beginPhase(Phase::Rest, restMin_ * 60);
}

void PracticeSession::finish()
{
    tick_.stop();
    engine_->stop();
    phase_ = Phase::Done;
    setRunning(false);
    emit changed();
}

void PracticeSession::onTick()
{
    if (clock_.elapsed() < phaseEndMs_)
    {
        emit changed();
        return;
    }

    if (phase_ == Phase::Practice)
    {
        if (round_ >= rounds_)
            finish();
        else
            enterRest();
    }
    else
    {
        ++round_;
        enterPractice();
    }
    emit transitioned();
}

void PracticeSession::setRunning(bool running)
{
    if (running_ == running)
        return;
    running_ = running;
    emit runningChanged(running);
}

/* ---- Configuración ------------------------------------------------------- */

void PracticeSession::setProfile(int index)
{
    const auto &list = profiles();
    if (index < 0 || index >= list.size())
        return;
    practiceMin_ = list[index].practiceMin;
    restMin_ = list[index].restMin;
    rounds_ = list[index].rounds;
    configChanged();
}

void PracticeSession::changePracticeMinutes(int delta)
{
    practiceMin_ = std::clamp(practiceMin_ + delta, kMinMinutes, kMaxPracticeMin);
    configChanged();
}

void PracticeSession::changeRestMinutes(int delta)
{
    restMin_ = std::clamp(restMin_ + delta, kMinMinutes, kMaxRestMin);
    configChanged();
}

void PracticeSession::changeRounds(int delta)
{
    rounds_ = std::clamp(rounds_ + delta, 1, kMaxRounds);
    configChanged();
}

void PracticeSession::configChanged()
{
    // Cambiar la configuración a mitad de sesión la reinicia.
    if (phase_ != Phase::Idle)
        stop();
    else
        emit changed();
}
