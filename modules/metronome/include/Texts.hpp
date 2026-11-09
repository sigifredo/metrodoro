#ifndef TEXTS_HPP
#define TEXTS_HPP

// own
#include <ClickConfig.hpp>

// qt
#include <QLocale>
#include <QString>

/// Texto de la interfaz según el idioma del sistema: español, inglés o francés (por defecto).
inline QString txt(const char *es, const char *en, const char *fr)
{
    switch (QLocale().language())
    {
    case QLocale::Spanish:
        return QString::fromUtf8(es);
    case QLocale::English:
        return QString::fromUtf8(en);
    default:
        return QString::fromUtf8(fr);
    }
}

/// Adapta los atajos escritos con «Ctrl» a la tecla del sistema: en macOS Qt::CTRL es Cmd.
inline QString nativeKeys(QString text)
{
#ifdef Q_OS_MACOS
    text.replace(QStringLiteral("Ctrl"), QStringLiteral("Cmd"));
#endif
    return text;
}

inline QString subdivisionLabel(Subdivision s)
{
    switch (s)
    {
    case Subdivision::Quarter:
        return txt("Negras", "Quarters", "Noires");
    case Subdivision::Eighth:
        return txt("Corcheas", "Eighths", "Croches");
    case Subdivision::Triplet:
        return txt("Tresillos", "Triplets", "Triolets");
    case Subdivision::Sixteenth:
        return txt("Semicorcheas", "Sixteenths", "Doubles");
    case Subdivision::Quintuplet:
        return txt("Quintillos", "Quintuplets", "Quintolets");
    case Subdivision::Sextuplet:
        return txt("Seisillos", "Sextuplets", "Sextolets");
    case Subdivision::Shuffle:
        return QStringLiteral("Shuffle");
    case Subdivision::Gallop:
        return txt("Galope", "Gallop", "Galop");
    case Subdivision::GallopRev:
        return txt("Galope inverso", "Reverse gallop", "Galop inversé");
    }
    return {};
}

inline QString soundLabel(ClickSound s)
{
    switch (s)
    {
    case ClickSound::Classic:
        return txt("Clásico", "Classic", "Classique");
    case ClickSound::Wood:
        return txt("Madera", "Wood", "Bois");
    case ClickSound::Click:
        return txt("Clic", "Click", "Clic");
    case ClickSound::Beep:
        return txt("Bip", "Beep", "Bip");
    case ClickSound::Hihat:
        return QStringLiteral("Hi-hat");
    case ClickSound::Cowbell:
        return txt("Cencerro", "Cowbell", "Cloche");
    }
    return {};
}

inline QString accentLabel(Accent a)
{
    switch (a)
    {
    case Accent::Mute:
        return txt("silencio", "mute", "silence");
    case Accent::Weak:
        return txt("débil", "weak", "faible");
    case Accent::Medium:
        return txt("medio", "medium", "moyen");
    case Accent::Strong:
        return txt("fuerte", "strong", "fort");
    }
    return {};
}

#endif // TEXTS_HPP
