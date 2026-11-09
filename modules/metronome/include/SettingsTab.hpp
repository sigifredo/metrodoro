#ifndef SETTINGSTAB_HPP
#define SETTINGSTAB_HPP

// qt
#include <QWidget>

class AppSettings;
class QCheckBox;
class QSlider;
class StatsStore;

/// Preferencias de la interfaz y el audio, borrado de datos y datos de la aplicación.
class SettingsTab : public QWidget
{
    Q_OBJECT

public:
    SettingsTab(AppSettings *settings, StatsStore *stats, QWidget *parent = nullptr);

    void refresh();

private:
    AppSettings *settings_;
    StatsStore *stats_;

    QCheckBox *pendulum_ = nullptr;
    QCheckBox *flash_ = nullptr;
    QSlider *volume_ = nullptr;
    QCheckBox *phaseSound_ = nullptr;
};

#endif // SETTINGSTAB_HPP
