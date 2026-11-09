// own
#include <ClickSynth.hpp>

// std
#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>

namespace
{
    constexpr double kTwoPi = 2.0 * std::numbers::pi;

    ClickSynth::Table render(ClickSound sound, int level)
    {
        constexpr int rate = ClickSynth::kSampleRate;
        std::mt19937 rng(7 + level);
        std::uniform_real_distribution<double> uniform(0.0, 1.0);

        double seconds = 0.0;
        switch (sound)
        {
        case ClickSound::Classic:
            seconds = 0.07;
            break;
        case ClickSound::Wood:
            seconds = 0.05;
            break;
        case ClickSound::Click:
            seconds = 0.03;
            break;
        case ClickSound::Beep:
            seconds = 0.06;
            break;
        case ClickSound::Hihat:
            seconds = 0.09;
            break;
        case ClickSound::Cowbell:
            seconds = 0.16;
            break;
        }

        const int n = static_cast<int>(seconds * rate);
        std::vector<double> out(n);
        double prevNoise = 0.0;
        for (int i = 0; i < n; ++i)
        {
            const double t = static_cast<double>(i) / rate;
            const double attack = std::min(1.0, t / 0.001);
            double v = 0.0;
            switch (sound)
            {
            case ClickSound::Classic:
            {
                // Las frecuencias originales: si (987,77 Hz) fuerte y sol (783,99 Hz) débil.
                constexpr double f[] = {987.767, 880.0, 783.991};
                v = std::sin(kTwoPi * f[level] * t) * std::exp(-t / 0.016);
                break;
            }
            case ClickSound::Wood:
            {
                constexpr double f[] = {1800.0, 1500.0, 1200.0};
                v = (std::sin(kTwoPi * f[level] * t) + 0.4 * std::sin(kTwoPi * f[level] * 2.4 * t)) * std::exp(-t / 0.007);
                break;
            }
            case ClickSound::Click:
            {
                constexpr double f[] = {3200.0, 2700.0, 2200.0};
                const double noise = uniform(rng) * 2 - 1;
                const double hp = noise - prevNoise;
                prevNoise = noise;
                v = (0.6 * hp + std::sin(kTwoPi * f[level] * t)) * std::exp(-t / 0.003);
                break;
            }
            case ClickSound::Beep:
            {
                constexpr double f[] = {880.0, 740.0, 620.0};
                const double fade = std::min(1.0, (seconds - t) / 0.004);
                v = (std::sin(kTwoPi * f[level] * t) + 0.3 * std::sin(kTwoPi * 3 * f[level] * t)) * fade;
                break;
            }
            case ClickSound::Hihat:
            {
                constexpr double decay[] = {0.03, 0.018, 0.01};
                const double noise = uniform(rng) * 2 - 1;
                const double hp = noise - prevNoise;
                prevNoise = noise;
                v = hp * std::exp(-t / decay[level]);
                break;
            }
            case ClickSound::Cowbell:
            {
                constexpr double k[] = {1.0, 0.94, 0.88};
                const double sq1 = std::sin(kTwoPi * 540 * k[level] * t) >= 0 ? 1.0 : -1.0;
                const double sq2 = std::sin(kTwoPi * 810 * k[level] * t) >= 0 ? 1.0 : -1.0;
                v = (sq1 + sq2) * 0.5 * std::exp(-t / 0.045);
                break;
            }
            }
            out[i] = attack * v;
        }

        double peak = 1e-9;
        for (double v : out)
            peak = std::max(peak, std::abs(v));

        ClickSynth::Table table(n);
        for (int i = 0; i < n; ++i)
            table[i] = static_cast<int16_t>(out[i] / peak * 0.9 * 32767);
        return table;
    }
}

namespace ClickSynth
{
    const std::array<Table, 3> &tables(ClickSound sound)
    {
        // Se calculan todos a la primera llamada (unos milisegundos) para no asignar memoria en el hilo de audio.
        static const std::array<std::array<Table, 3>, kClickSoundCount> all = []
        {
            std::array<std::array<Table, 3>, kClickSoundCount> result;
            for (int s = 0; s < kClickSoundCount; ++s)
            {
                for (int level = 0; level < 3; ++level)
                    result[s][level] = render(static_cast<ClickSound>(s), level);
            }
            return result;
        }();
        return all[static_cast<int>(sound)];
    }

    Table chime(bool rising, int notes)
    {
        const double rising3[] = {784.0, 1046.5, 1318.5};
        const double falling3[] = {1046.5, 784.0, 659.3};
        const double *freqs = rising ? rising3 : falling3;
        notes = std::clamp(notes, 1, 3);

        const int each = static_cast<int>(0.18 * kSampleRate);
        const int tail = static_cast<int>(0.25 * kSampleRate);
        Table out(each * notes + tail, 0);
        for (int k = 0; k < notes; ++k)
        {
            for (int i = 0; i < each + tail; ++i)
            {
                const int idx = k * each + i;
                if (idx >= static_cast<int>(out.size()))
                    break;
                const double t = static_cast<double>(i) / kSampleRate;
                const double v = std::sin(kTwoPi * freqs[k] * t) * std::exp(-t / 0.12) * std::min(1.0, t / 0.004) * 0.6;
                out[idx] = static_cast<int16_t>(std::clamp(out[idx] + static_cast<int>(v * 32767), -32767, 32767));
            }
        }
        return out;
    }
}
