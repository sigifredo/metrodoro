// own
#include <AppSettings.hpp>
#include <Controls.hpp>
#include <SettingsTab.hpp>
#include <StatsStore.hpp>
#include <Texts.hpp>

// qt
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

namespace
{
    QLabel *note(const QString &text, QWidget *parent)
    {
        auto *label = new QLabel(text, parent);
        label->setObjectName(QStringLiteral("hint"));
        label->setWordWrap(true);
        return label;
    }
}

SettingsTab::SettingsTab(AppSettings *settings, StatsStore *stats, QWidget *parent)
    : QWidget(parent), settings_(settings), stats_(stats)
{
    auto *page = new QVBoxLayout(this);
    page->setContentsMargins(20, 20, 20, 20);
    page->setSpacing(16);
    QVBoxLayout *body = nullptr;

    page->addWidget(makeCard(txt("Apariencia", "Appearance", "Apparence"), this, &body));
    pendulum_ = new QCheckBox(txt("Péndulo", "Pendulum", "Pendule"), this);
    body->addWidget(makeSwitchRow(pendulum_, txt("Muestra un péndulo que cruza en cada tiempo", "Shows a pendulum that crosses on every beat", "Affiche un pendule qui traverse à chaque temps"), this));
    flash_ = new QCheckBox(txt("Destello de pantalla", "Screen flash", "Flash d'écran"), this);
    body->addWidget(makeSwitchRow(flash_, txt("Se ve el pulso sin oírlo", "See the beat without hearing it", "On voit le temps sans l'entendre"), this));

    page->addWidget(makeCard(txt("Audio", "Audio", "Audio"), this, &body));
    body->addWidget(new QLabel(txt("Volumen del metrónomo", "Metronome volume", "Volume du métronome"), this));
    volume_ = new QSlider(Qt::Horizontal, this);
    volume_->setRange(10, 100);
    body->addWidget(volume_);
    phaseSound_ = new QCheckBox(txt("Aviso sonoro al cambiar de fase", "Sound alert on phase change", "Signal sonore au changement de phase"), this);
    body->addWidget(makeSwitchRow(phaseSound_,
                                  txt("Un tono propio al pasar de práctica a descanso y al terminar", "Its own tone when going from practice to break and at the end", "Un signal propre au passage de la pratique à la pause et à la fin"),
                                  this));

    pauseOnUnplug_ = new QCheckBox(txt("Pausar al perder la salida de audio", "Pause when the audio output is lost", "Mettre en pause si la sortie audio disparaît"), this);
    body->addWidget(makeSwitchRow(pauseOnUnplug_,
                                  txt("Evita que suene por los altavoces sin querer al desconectar los auriculares", "Keeps it from playing through the speakers by accident when headphones are unplugged", "Évite que le son passe par les haut-parleurs par erreur quand le casque est débranché"),
                                  this));
    keepAwake_ = new QCheckBox(txt("Evitar la suspensión mientras suena", "Prevent sleep while playing", "Empêcher la mise en veille pendant la lecture"), this);
    body->addWidget(makeSwitchRow(keepAwake_,
                                  txt("Solo en Linux; en otros sistemas no tiene efecto", "Linux only; it has no effect on other systems", "Linux uniquement ; sans effet sur les autres systèmes"),
                                  this));

    page->addWidget(makeCard(txt("Datos", "Data", "Données"), this, &body));
    auto *clearStats = new QPushButton(txt("Borrar estadísticas", "Clear statistics", "Effacer les statistiques"), this);
    auto *reset = new QPushButton(txt("Restablecer ajustes", "Reset settings", "Réinitialiser les réglages"), this);
    for (auto *button : {clearStats, reset})
    {
        button->setObjectName(QStringLiteral("secondary"));
        button->setCursor(Qt::PointingHandCursor);
        button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        body->addWidget(button);
    }

    page->addWidget(makeCard(txt("Teclado", "Keyboard", "Clavier"), this, &body));
    body->addWidget(note(txt("Espacio: tocar o pausar\nT: marcar el tempo (TAP)\nCtrl+↑ / Ctrl+↓: tempo ±1\nCtrl+Mayús+↑ / ↓: tempo ±5\nCtrl+S: guardar como preajuste\nCtrl+1, 2, 3, 4: cambiar de pestaña",
                             "Space: play or pause\nT: tap the tempo\nCtrl+↑ / Ctrl+↓: tempo ±1\nCtrl+Shift+↑ / ↓: tempo ±5\nCtrl+S: save as preset\nCtrl+1, 2, 3, 4: switch tab",
                             "Espace : lecture ou pause\nT : marquer le tempo (TAP)\nCtrl+↑ / Ctrl+↓ : tempo ±1\nCtrl+Maj+↑ / ↓ : tempo ±5\nCtrl+S : enregistrer comme préréglage\nCtrl+1, 2, 3, 4 : changer d'onglet"),
                         this));

    page->addWidget(makeCard(txt("Acerca de", "About", "À propos"), this, &body));
    body->addWidget(new QLabel(QStringLiteral("Metrodoro %1").arg(QStringLiteral(METRODORO_VERSION)), this));
    body->addWidget(note(txt("Metrónomo y práctica con descansos. Sin anuncios, sin red y sin telemetría: todo se queda en tu equipo.",
                             "Metronome and practice with breaks. No ads, no network and no telemetry: everything stays on your computer.",
                             "Métronome et pratique avec pauses. Sans publicité, sans réseau et sans télémétrie : tout reste sur votre ordinateur."),
                         this));
    body->addWidget(note(txt("Tipografías Inter y Hack, con sus licencias en la carpeta de fuentes del proyecto.",
                             "Inter and Hack typefaces, with their licenses in the project's fonts folder.",
                             "Polices Inter et Hack, avec leurs licences dans le dossier des polices du projet."),
                         this));
    page->addStretch();

    connect(pendulum_, &QCheckBox::toggled, settings_, &AppSettings::setPendulum);
    connect(flash_, &QCheckBox::toggled, settings_, &AppSettings::setFlash);
    connect(volume_, &QSlider::valueChanged, this, [this](int v)
            { settings_->setVolume(v / 100.0f); });
    connect(phaseSound_, &QCheckBox::toggled, settings_, &AppSettings::setPhaseSound);
    connect(pauseOnUnplug_, &QCheckBox::toggled, settings_, &AppSettings::setPauseOnUnplug);
    connect(keepAwake_, &QCheckBox::toggled, settings_, &AppSettings::setKeepAwake);
    connect(clearStats, &QPushButton::clicked, this, [this]()
            {
                if (confirmDialog(this, txt("¿Borrar el tiempo practicado?", "Clear the practiced time?", "Effacer le temps pratiqué ?")))
                    stats_->clear(); });
    connect(reset, &QPushButton::clicked, this, [this]()
            {
                if (confirmDialog(this, txt("¿Restablecer los ajustes?", "Reset the settings?", "Réinitialiser les réglages ?")))
                    settings_->reset(); });

    refresh();
}

void SettingsTab::refresh()
{
    const auto sync = [](QCheckBox *box, bool value)
    {
        const QSignalBlocker blocker(box);
        box->setChecked(value);
    };
    sync(pendulum_, settings_->pendulum());
    sync(flash_, settings_->flash());
    sync(phaseSound_, settings_->phaseSound());
    sync(pauseOnUnplug_, settings_->pauseOnUnplug());
    sync(keepAwake_, settings_->keepAwake());
    const QSignalBlocker blocker(volume_);
    volume_->setValue(qRound(settings_->volume() * 100));
}
