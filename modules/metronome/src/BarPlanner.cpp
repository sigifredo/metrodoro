// own
#include <BarPlanner.hpp>

// std
#include <algorithm>

BarPlanner::BarPlanner()
    : random_(std::random_device{}())
{
}

void BarPlanner::reset(const ClickConfig &config)
{
    countInLeft_ = config.countInBars;
    played_ = 0;
    baseBpm_ = config.bpm;
    bpm_ = config.bpm;
}

bool BarPlanner::rebase(int newBpm)
{
    if (newBpm == baseBpm_)
        return false;
    baseBpm_ = newBpm;
    bpm_ = newBpm;
    played_ = 0;
    return true;
}

BarPlanner::Bar BarPlanner::next(const ClickConfig &config)
{
    if (countInLeft_ > 0)
    {
        --countInLeft_;
        return {true, false, bpm_};
    }

    const TrainerConfig &t = config.trainer;
    if (t.enabled && played_ > 0 && played_ % t.everyBars == 0)
    {
        if (t.targetBpm >= baseBpm_)
            bpm_ = std::min(bpm_ + t.stepBpm, std::max(t.targetBpm, baseBpm_));
        else
            bpm_ = std::max(bpm_ - t.stepBpm, t.targetBpm);
    }

    const GapConfig &g = config.gap;
    bool muted = false;
    if (g.enabled && played_ > 0)
    {
        if (g.random)
        {
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            muted = dist(random_) < static_cast<float>(g.muteBars) / static_cast<float>(g.playBars + g.muteBars);
        }
        else
        {
            muted = played_ % (g.playBars + g.muteBars) >= g.playBars;
        }
    }
    ++played_;
    return {false, muted, bpm_};
}
