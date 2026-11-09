#ifndef CLICKSYNTH_HPP
#define CLICKSYNTH_HPP

// own
#include <ClickConfig.hpp>

// std
#include <array>
#include <cstdint>
#include <vector>

/// Sintetiza los timbres muestra a muestra, sin archivos de audio.
namespace ClickSynth
{
    inline constexpr int kSampleRate = 44100;

    enum Level
    {
        High = 0, // tiempo fuerte
        Mid = 1,  // tiempo medio
        Low = 2   // tiempo débil y subdivisiones
    };

    using Table = std::vector<int16_t>;

    /// Tres variantes de altura (fuerte, medio, débil) del timbre. Se calculan una sola vez.
    const std::array<Table, 3> &tables(ClickSound sound);

    /// Aviso de cambio de fase de la sesión de práctica: notas ascendentes o descendentes.
    Table chime(bool rising, int notes = 2);
}

#endif // CLICKSYNTH_HPP
