#ifndef BARPLANNER_HPP
#define BARPLANNER_HPP

// own
#include <ClickConfig.hpp>

// std
#include <random>

/**
 * Decide, compás a compás, qué pasa con la cuenta previa, el entrenador de velocidad y los compases mudos.
 * No depende de Qt Multimedia: la usa el hilo de audio.
 */
class BarPlanner
{
public:
    struct Bar
    {
        bool countIn = false;
        bool muted = false;
        int bpm = 0;
    };

    BarPlanner();

    /// Compases tocados sin contar la cuenta previa.
    int playedBars() const { return played_; }

    void reset(const ClickConfig &config);
    /// El usuario cambió el tempo base: el entrenador parte de ahí otra vez. Devuelve si hubo cambio.
    bool rebase(int newBpm);
    Bar next(const ClickConfig &config);

private:
    int countInLeft_ = 0;
    int played_ = 0;
    int bpm_ = 0;
    int baseBpm_ = 0;
    std::mt19937 random_;
};

#endif // BARPLANNER_HPP
