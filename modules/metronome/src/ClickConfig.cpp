// own
#include <ClickConfig.hpp>

// qt
#include <QJsonArray>

// std
#include <algorithm>

namespace
{
    constexpr const char *kAccentNames[] = {"MUTE", "WEAK", "MEDIUM", "STRONG"};
    constexpr const char *kSubdivisionNames[] = {"QUARTER", "EIGHTH", "TRIPLET", "SIXTEENTH", "QUINTUPLET",
                                                 "SEXTUPLET", "SHUFFLE", "GALLOP", "GALLOP_REV"};
    constexpr const char *kSoundNames[] = {"CLASSIC", "WOOD", "CLICK", "BEEP", "HIHAT", "COWBELL"};

    template <typename E, std::size_t N>
    E enumFromName(const QString &name, const char *const (&names)[N], E fallback)
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            if (name == QLatin1String(names[i]))
                return static_cast<E>(i);
        }
        return fallback;
    }

    struct TempoRange
    {
        int min;
        int max;
        const char *name;
    };

    constexpr TempoRange kTempos[] = {
        {10, 39, "Larghissimo"}, {40, 60, "Largo"}, {52, 68, "Lento"},
        {60, 80, "Adagio"}, {76, 100, "Andante"}, {88, 112, "Moderato"},
        {100, 128, "Allegretto"}, {112, 160, "Allegro"}, {138, 142, "Vivace"},
        {140, 200, "Presto"}, {188, 260, "Prestissimo"}};
}

float accentGain(Accent accent)
{
    switch (accent)
    {
    case Accent::Mute:
        return 0.0f;
    case Accent::Weak:
        return 0.5f;
    case Accent::Medium:
        return 0.75f;
    case Accent::Strong:
        return 1.0f;
    }
    return 0.0f;
}

Accent nextAccent(Accent accent)
{
    switch (accent)
    {
    case Accent::Strong:
        return Accent::Medium;
    case Accent::Medium:
        return Accent::Weak;
    case Accent::Weak:
        return Accent::Mute;
    case Accent::Mute:
        return Accent::Strong;
    }
    return Accent::Strong;
}

const QList<bool> &subdivisionMask(Subdivision subdivision)
{
    static const QList<bool> masks[kSubdivisionCount] = {
        {true},
        {true, true},
        {true, true, true},
        {true, true, true, true},
        {true, true, true, true, true},
        {true, true, true, true, true, true},
        {true, false, true},
        {true, false, true, true},
        {true, true, false, true}};
    return masks[static_cast<int>(subdivision)];
}

ClickConfig::ClickConfig()
    : accents(defaultAccents(4, 4))
{
}

ClickConfig ClickConfig::withBeats(int n) const
{
    const int count = std::clamp(n, kMinBeats, kMaxBeats);
    if (count == beats)
        return *this;
    const QList<Accent> fresh = defaultAccents(count, unit);
    ClickConfig result = *this;
    result.beats = count;
    result.groups.clear();
    result.accents.clear();
    for (int i = 0; i < count; ++i)
        result.accents << (i < accents.size() && i != 0 ? accents[i] : fresh[i]);
    return result;
}

ClickConfig ClickConfig::withSignature(int newBeats, int newUnit) const
{
    ClickConfig result = *this;
    result.beats = std::clamp(newBeats, kMinBeats, kMaxBeats);
    result.unit = newUnit;
    result.groups.clear();
    result.accents = defaultAccents(result.beats, newUnit);
    return result;
}

ClickConfig ClickConfig::withGroups(const QList<int> &newGroups) const
{
    int total = 0;
    for (int g : newGroups)
        total += g;
    ClickConfig result = *this;
    if (newGroups.isEmpty() || total < kMinBeats || total > kMaxBeats)
        return result;
    result.beats = total;
    result.groups = newGroups;
    result.accents = defaultAccents(total, unit, newGroups);
    return result;
}

QList<int> ClickConfig::effectiveGroups() const
{
    return groups.isEmpty() ? beatGroups(beats, unit) : groups;
}

ClickConfig ClickConfig::withBpm(int value) const
{
    ClickConfig result = *this;
    result.bpm = std::clamp(value, kMinBpm, kMaxBpm);
    return result;
}

QJsonObject ClickConfig::toJson() const
{
    QJsonArray accentArray;
    for (Accent a : accents)
        accentArray << QLatin1String(kAccentNames[static_cast<int>(a)]);
    return {
        {"bpm", bpm},
        {"beats", beats},
        {"unit", unit},
        {"accents", accentArray},
        {"groups", QJsonArray::fromVariantList(QVariantList(groups.cbegin(), groups.cend()))},
        {"sub", QLatin1String(kSubdivisionNames[static_cast<int>(subdivision)])},
        {"subVol", static_cast<double>(subVolume)},
        {"sound", QLatin1String(kSoundNames[static_cast<int>(sound)])},
        {"countIn", countInBars},
        {"tEnabled", trainer.enabled},
        {"tStep", trainer.stepBpm},
        {"tEvery", trainer.everyBars},
        {"tTarget", trainer.targetBpm},
        {"gEnabled", gap.enabled},
        {"gPlay", gap.playBars},
        {"gMute", gap.muteBars},
        {"gRandom", gap.random}};
}

ClickConfig ClickConfig::fromJson(const QJsonObject &o)
{
    const ClickConfig d;
    ClickConfig c;
    c.beats = std::clamp(o.value("beats").toInt(d.beats), kMinBeats, kMaxBeats);
    const int unit = o.value("unit").toInt(d.unit);
    c.unit = kUnits.contains(unit) ? unit : d.unit;

    const QJsonArray saved = o.value("accents").toArray();
    if (saved.size() == c.beats)
    {
        c.accents.clear();
        for (const QJsonValue &v : saved)
            c.accents << enumFromName(v.toString(), kAccentNames, Accent::Weak);
    }
    else
    {
        c.accents = defaultAccents(c.beats, c.unit);
    }

    QList<int> savedGroups;
    int groupTotal = 0;
    for (const QJsonValue &v : o.value("groups").toArray())
    {
        savedGroups << v.toInt();
        groupTotal += v.toInt();
    }
    const bool validGroups = !savedGroups.isEmpty() && groupTotal == c.beats && !savedGroups.contains(0) &&
                             std::all_of(savedGroups.cbegin(), savedGroups.cend(), [](int g)
                                         { return g > 0; });
    if (validGroups)
        c.groups = savedGroups;
    if (saved.size() != c.beats)
        c.accents = defaultAccents(c.beats, c.unit, c.groups);

    c.bpm = std::clamp(o.value("bpm").toInt(d.bpm), kMinBpm, kMaxBpm);
    c.subdivision = enumFromName(o.value("sub").toString(), kSubdivisionNames, d.subdivision);
    c.subVolume = std::clamp(static_cast<float>(o.value("subVol").toDouble(d.subVolume)), 0.0f, 1.0f);
    c.sound = enumFromName(o.value("sound").toString(), kSoundNames, d.sound);
    const int countIn = o.value("countIn").toInt(d.countInBars);
    c.countInBars = kCountIns.contains(countIn) ? countIn : 0;

    c.trainer.enabled = o.value("tEnabled").toBool(false);
    c.trainer.stepBpm = std::clamp(o.value("tStep").toInt(d.trainer.stepBpm), 1, 20);
    c.trainer.everyBars = std::clamp(o.value("tEvery").toInt(d.trainer.everyBars), 1, 32);
    c.trainer.targetBpm = std::clamp(o.value("tTarget").toInt(d.trainer.targetBpm), kMinBpm, kMaxBpm);

    c.gap.enabled = o.value("gEnabled").toBool(false);
    c.gap.playBars = std::clamp(o.value("gPlay").toInt(d.gap.playBars), 1, 16);
    c.gap.muteBars = std::clamp(o.value("gMute").toInt(d.gap.muteBars), 1, 16);
    c.gap.random = o.value("gRandom").toBool(false);
    return c;
}

QList<int> beatGroups(int beats, int unit)
{
    if (unit == 8 && beats >= 6 && beats % 3 == 0)
        return QList<int>(beats / 3, 3);
    if (unit == 8 && beats == 7)
        return {2, 2, 3};
    if (beats == 5)
        return {3, 2};
    if (beats == 7)
        return {4, 3};
    return {beats};
}

QList<Accent> defaultAccents(int beats, int unit, const QList<int> &custom)
{
    QList<Accent> result(beats, Accent::Weak);
    int start = 0;
    int customTotal = 0;
    for (int g : custom)
        customTotal += g;
    const QList<int> groups = !custom.isEmpty() && customTotal == beats ? custom : beatGroups(beats, unit);
    for (int i = 0; i < groups.size(); ++i)
    {
        if (start < beats)
            result[start] = i == 0 ? Accent::Strong : Accent::Medium;
        start += groups[i];
    }
    return result;
}

QStringList tempoNames(int bpm)
{
    QStringList names;
    for (const auto &tempo : kTempos)
    {
        if (tempo.min <= bpm && bpm <= tempo.max)
            names << QString::fromLatin1(tempo.name);
    }
    return names;
}

std::optional<QList<int>> parseGroups(const QString &text)
{
    QList<int> groups;
    int total = 0;
    for (const QString &part : text.split(QLatin1Char('+'), Qt::SkipEmptyParts))
    {
        bool ok = false;
        const int n = part.trimmed().toInt(&ok);
        if (!ok || n < 1)
            return std::nullopt;
        groups << n;
        total += n;
    }
    if (groups.isEmpty() || total > ClickConfig::kMaxBeats)
        return std::nullopt;
    return groups;
}

QString groupsText(const QList<int> &groups)
{
    QStringList parts;
    for (int g : groups)
        parts << QString::number(g);
    return parts.join(QLatin1Char('+'));
}
