#ifndef PHASECHIME_HPP
#define PHASECHIME_HPP

// qt
#include <QObject>

/// Aviso propio (no el pitido del sistema) cuando la sesión de práctica cambia de fase.
class PhaseChime : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    /// `rising` sube de tono (empieza a practicar); `notes` = 3 marca el final de la sesión.
    void play(bool rising, int notes, float volume);
};

#endif // PHASECHIME_HPP
