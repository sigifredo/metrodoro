#ifndef METRONOMETAB_HPP
#define METRONOMETAB_HPP

// qt
#include <QWidget>

class AppSettings;
class ChipRow;
class MetronomeEngine;
class QCheckBox;
class QSlider;
class StepperRow;
class SubdivisionSelector;

/// Lo que se cambia poco durante el estudio: compás, subdivisión, sonido y ayudas.
class MetronomeTab : public QWidget
{
    Q_OBJECT

public:
    MetronomeTab(MetronomeEngine *engine, AppSettings *settings, QWidget *parent = nullptr);

    void refresh();

private:
    MetronomeEngine *engine_;
    AppSettings *settings_;

    StepperRow *beats_ = nullptr;
    ChipRow *units_ = nullptr;
    ChipRow *signatures_ = nullptr;
    SubdivisionSelector *subdivisions_ = nullptr;
    ChipRow *sounds_ = nullptr;
    QSlider *subVolume_ = nullptr;
    ChipRow *countIn_ = nullptr;
    QCheckBox *flash_ = nullptr;
};

#endif // METRONOMETAB_HPP
