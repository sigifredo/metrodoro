#ifndef CLICKCONFIG_HPP
#define CLICKCONFIG_HPP

// qt
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

// std
#include <optional>

/// Intensidad de un tiempo del compás.
enum class Accent
{
    Mute,
    Weak,
    Medium,
    Strong
};

float accentGain(Accent accent);
/// Ciclo al pulsar un tiempo: fuerte, medio, débil, silencio.
Accent nextAccent(Accent accent);

/// División de cada tiempo. La máscara marca qué huecos suenan (el primero es el propio tiempo).
enum class Subdivision
{
    Quarter,
    Eighth,
    Triplet,
    Sixteenth,
    Quintuplet,
    Sextuplet,
    Shuffle,
    Gallop,
    GallopRev
};

inline constexpr int kSubdivisionCount = 9;

const QList<bool> &subdivisionMask(Subdivision subdivision);
inline int subdivisionSlots(Subdivision subdivision) { return static_cast<int>(subdivisionMask(subdivision).size()); }

enum class ClickSound
{
    Classic,
    Wood,
    Click,
    Beep,
    Hihat,
    Cowbell
};

inline constexpr int kClickSoundCount = 6;

/// Sube (o baja) el tempo `stepBpm` cada `everyBars` compases hasta `targetBpm`.
struct TrainerConfig
{
    bool enabled = false;
    int stepBpm = 5;
    int everyBars = 4;
    int targetBpm = 160;

    bool operator==(const TrainerConfig &) const = default;
};

/// Suena `playBars` compases y calla `muteBars`; con `random` los silencios caen al azar.
struct GapConfig
{
    bool enabled = false;
    int playBars = 2;
    int muteBars = 1;
    bool random = false;

    bool operator==(const GapConfig &) const = default;
};

/// Todo lo que define cómo suena el metrónomo. Es un valor: el hilo de audio trabaja con copias.
struct ClickConfig
{
    static constexpr int kMinBpm = 20;
    static constexpr int kMaxBpm = 300;
    static constexpr int kMinBeats = 1;
    static constexpr int kMaxBeats = 16;
    static inline const QList<int> kUnits = {2, 4, 8, 16};
    static inline const QList<int> kCountIns = {0, 1, 2, 4};

    int bpm = 100;
    int beats = 4;
    int unit = 4;
    QList<Accent> accents;
    /// Tamaños de los grupos de tiempos (5+7, 2+2+3...). Vacío: los habituales del compás.
    QList<int> groups;
    Subdivision subdivision = Subdivision::Quarter;
    float subVolume = 0.5f;
    ClickSound sound = ClickSound::Classic;
    int countInBars = 0;
    TrainerConfig trainer;
    GapConfig gap;

    ClickConfig();

    bool operator==(const ClickConfig &) const = default;

    /// Cambia el número de tiempos conservando los acentos que ya existen.
    ClickConfig withBeats(int n) const;
    /// Fija el compás completo y reinicia los acentos agrupados según el compás.
    ClickConfig withSignature(int beats, int unit) const;
    /// Compás de tamaño mixto: los tiempos son la suma de los grupos y cada grupo empieza con acento medio.
    ClickConfig withGroups(const QList<int> &groups) const;
    ClickConfig withBpm(int value) const;
    /// Los grupos en uso: los personalizados o los habituales del compás.
    QList<int> effectiveGroups() const;

    QJsonObject toJson() const;
    static ClickConfig fromJson(const QJsonObject &object);
};

/// Tamaños de los grupos de tiempos de un compás: 6/8 es 3+3, 7/8 es 2+2+3, 5/4 es 3+2.
QList<int> beatGroups(int beats, int unit);
/// Fuerte en el primer tiempo, medio al inicio de cada grupo y débil en el resto.
/// Con `groups` que sumen `beats` se usan esos grupos en lugar de los habituales.
QList<Accent> defaultAccents(int beats, int unit, const QList<int> &groups = {});
/// Lee grupos escritos como "5+7". Devuelve nada si hay algo inválido o suman más de 16 tiempos.
std::optional<QList<int>> parseGroups(const QString &text);
/// Escribe los grupos como "5+7".
QString groupsText(const QList<int> &groups);
/// Nombres italianos del tempo; los rangos se solapan, así que puede haber varios.
QStringList tempoNames(int bpm);

#endif // CLICKCONFIG_HPP
