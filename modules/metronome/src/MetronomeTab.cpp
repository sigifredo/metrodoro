// own
#include <AppSettings.hpp>
#include <Controls.hpp>
#include <MetronomeEngine.hpp>
#include <MetronomeTab.hpp>
#include <SubdivisionSelector.hpp>
#include <Texts.hpp>

// qt
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

namespace
{
    // Compases habituales con sus acentos agrupados (6/8 es 3+3, 7/8 es 2+2+3...).
    const QList<QPair<int, int>> kSignatures = {{2, 4}, {3, 4}, {4, 4}, {5, 4}, {6, 8}, {7, 8}, {9, 8}, {12, 8}};
}

MetronomeTab::MetronomeTab(MetronomeEngine *engine, AppSettings *settings, QWidget *parent)
    : QWidget(parent), engine_(engine), settings_(settings)
{
    auto *page = new QVBoxLayout(this);
    page->setContentsMargins(20, 20, 20, 20);
    page->setSpacing(16);

    QVBoxLayout *body = nullptr;

    // Compás
    page->addWidget(makeCard(txt("Compás", "Time signature", "Mesure"), this, &body));
    beats_ = new StepperRow(txt("Tiempos", "Beats", "Temps"), this);
    body->addWidget(beats_);
    body->addWidget(new QLabel(txt("Unidad", "Beat unit", "Unité"), this));
    QStringList unitLabels;
    for (int u : ClickConfig::kUnits)
        unitLabels << QString::number(u);
    units_ = new ChipRow(unitLabels, this);
    body->addWidget(units_);
    body->addWidget(new QLabel(txt("Compases habituales", "Common signatures", "Mesures courantes"), this));
    QStringList signatureLabels;
    for (const auto &s : kSignatures)
        signatureLabels << QStringLiteral("%1/%2").arg(s.first).arg(s.second);
    signatures_ = new ChipRow(signatureLabels, this);
    body->addWidget(signatures_);
    body->addWidget(new QLabel(txt("Grupos de tiempos (compás mixto)", "Beat groups (mixed meter)", "Groupes de temps (mesure mixte)"), this));
    groups_ = new QLineEdit(this);
    groups_->setMaximumWidth(180);
    groups_->setToolTip(txt("Por ejemplo 5+7 o 2+2+3. Vacío: grupos habituales del compás.",
                            "For example 5+7 or 2+2+3. Empty: the usual groups for the signature.",
                            "Par exemple 5+7 ou 2+2+3. Vide : groupes habituels de la mesure."));
    body->addWidget(groups_);
    auto *groupsHint = new QLabel(txt("Cada grupo empieza con acento medio. Ej.: 5+7 o 2+2+3; vacío es automático.",
                                      "Each group starts with a medium accent. E.g. 5+7 or 2+2+3; empty is automatic.",
                                      "Chaque groupe commence par un accent moyen. Ex. 5+7 ou 2+2+3 ; vide est automatique."),
                                  this);
    groupsHint->setObjectName(QStringLiteral("hint"));
    groupsHint->setWordWrap(true);
    body->addWidget(groupsHint);

    // Subdivisión
    page->addWidget(makeCard(txt("Subdivisión", "Subdivision", "Subdivision"), this, &body));
    subdivisions_ = new SubdivisionSelector(this);
    body->addWidget(subdivisions_);

    // Sonido y ayudas
    page->addWidget(makeCard(txt("Sonido y ayudas", "Sound and aids", "Son et aides"), this, &body));
    QStringList soundLabels;
    for (int i = 0; i < kClickSoundCount; ++i)
        soundLabels << soundLabel(static_cast<ClickSound>(i));
    sounds_ = new ChipRow(soundLabels, this);
    body->addWidget(sounds_);
    body->addWidget(new QLabel(txt("Volumen de la subdivisión", "Subdivision volume", "Volume de la subdivision"), this));
    subVolume_ = new QSlider(Qt::Horizontal, this);
    subVolume_->setRange(0, 100);
    body->addWidget(subVolume_);
    body->addWidget(new QLabel(txt("Cuenta previa", "Count-in", "Décompte"), this));
    QStringList countLabels;
    for (int n : ClickConfig::kCountIns)
    {
        countLabels << (n == 0 ? txt("Sin cuenta", "None", "Aucun")
                               : (n == 1 ? txt("1 compás", "1 bar", "1 mesure")
                                         : txt("%1 compases", "%1 bars", "%1 mesures").arg(n)));
    }
    countIn_ = new ChipRow(countLabels, this);
    body->addWidget(countIn_);
    flash_ = new QCheckBox(txt("Destello de pantalla", "Screen flash", "Flash d'écran"), this);
    body->addWidget(makeSwitchRow(flash_, txt("Se ve el pulso sin oírlo", "See the beat without hearing it", "On voit le temps sans l'entendre"), this));

    page->addStretch();

    connect(beats_, &StepperRow::stepped, this, [this](int d)
            { engine_->edit([d](const ClickConfig &c)
                            { return c.withBeats(c.beats + d); }); });
    connect(units_, &ChipRow::activated, this, [this](int i)
            { engine_->edit([unit = ClickConfig::kUnits[i]](const ClickConfig &c)
                            {
                                ClickConfig next = c;
                                next.unit = unit;
                                next.accents = defaultAccents(c.beats, unit, c.groups);
                                return next; }); });
    connect(signatures_, &ChipRow::activated, this, [this](int i)
            { engine_->edit([s = kSignatures[i]](const ClickConfig &c)
                            { return c.withSignature(s.first, s.second); }); });
    connect(groups_, &QLineEdit::editingFinished, this, [this]()
            {
                const QString text = groups_->text().trimmed();
                if (text.isEmpty())
                {
                    engine_->edit([](const ClickConfig &c)
                                  { return c.withSignature(c.beats, c.unit); });
                }
                else if (const auto parsed = parseGroups(text))
                {
                    engine_->edit([g = *parsed](const ClickConfig &c)
                                  { return c.withGroups(g); });
                }
                const QList<int> now = engine_->config().groups;
                groups_->setText(now.isEmpty() ? QString() : groupsText(now));
                refresh(); });
    connect(subdivisions_, &SubdivisionSelector::selected, this, [this](Subdivision s)
            { engine_->edit([s](const ClickConfig &c)
                            {
                                ClickConfig next = c;
                                next.subdivision = s;
                                return next; }); });
    connect(sounds_, &ChipRow::activated, this, [this](int i)
            { engine_->edit([i](const ClickConfig &c)
                            {
                                ClickConfig next = c;
                                next.sound = static_cast<ClickSound>(i);
                                return next; }); });
    connect(subVolume_, &QSlider::valueChanged, this, [this](int v)
            { engine_->edit([v](const ClickConfig &c)
                            {
                                ClickConfig next = c;
                                next.subVolume = v / 100.0f;
                                return next; }); });
    connect(countIn_, &ChipRow::activated, this, [this](int i)
            { engine_->edit([n = ClickConfig::kCountIns[i]](const ClickConfig &c)
                            {
                                ClickConfig next = c;
                                next.countInBars = n;
                                return next; }); });
    connect(flash_, &QCheckBox::toggled, settings_, &AppSettings::setFlash);

    refresh();
}

void MetronomeTab::refresh()
{
    const ClickConfig &c = engine_->config();
    beats_->setText(QString::number(c.beats));
    units_->setCurrent(static_cast<int>(ClickConfig::kUnits.indexOf(c.unit)));
    int signature = -1;
    for (int i = 0; i < kSignatures.size(); ++i)
    {
        if (kSignatures[i].first == c.beats && kSignatures[i].second == c.unit)
            signature = i;
    }
    signatures_->setCurrent(c.groups.isEmpty() ? signature : -1);
    if (!groups_->hasFocus())
        groups_->setText(c.groups.isEmpty() ? QString() : groupsText(c.groups));
    groups_->setPlaceholderText(groupsText(beatGroups(c.beats, c.unit)));
    subdivisions_->setSelection(c.subdivision);
    sounds_->setCurrent(static_cast<int>(c.sound));
    countIn_->setCurrent(static_cast<int>(ClickConfig::kCountIns.indexOf(c.countInBars)));
    {
        const QSignalBlocker blocker(subVolume_);
        subVolume_->setValue(qRound(c.subVolume * 100));
    }
    subVolume_->setEnabled(c.subdivision != Subdivision::Quarter);
    {
        const QSignalBlocker blocker(flash_);
        flash_->setChecked(settings_->flash());
    }
}
