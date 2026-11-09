// own
#include <Controls.hpp>
#include <LibraryTab.hpp>
#include <MetronomeEngine.hpp>
#include <PracticeSession.hpp>
#include <Texts.hpp>

// qt
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace
{
    constexpr int kUndoMs = 8000;

    void clearLayout(QLayout *layout)
    {
        while (QLayoutItem *item = layout->takeAt(0))
        {
            if (QWidget *w = item->widget())
                w->deleteLater();
            delete item;
        }
    }

    QPushButton *makeButton(const QString &text, const char *kind, QWidget *parent)
    {
        auto *button = new QPushButton(text, parent);
        button->setObjectName(QString::fromLatin1(kind));
        button->setCursor(Qt::PointingHandCursor);
        button->setFocusPolicy(Qt::NoFocus);
        return button;
    }

    /// Fila con título, línea de detalle y un hueco a la derecha para botones.
    QHBoxLayout *makeRow(QVBoxLayout *list, QWidget *parent, const QString &title, const QString &detail)
    {
        auto *row = new QFrame(parent);
        row->setObjectName(QStringLiteral("row"));
        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(14, 10, 10, 10);
        layout->setSpacing(6);
        auto *text = new QVBoxLayout;
        text->setSpacing(2);
        auto *titleLabel = new QLabel(title, row);
        titleLabel->setObjectName(QStringLiteral("strong"));
        text->addWidget(titleLabel);
        if (!detail.isEmpty())
        {
            auto *detailLabel = new QLabel(detail, row);
            detailLabel->setObjectName(QStringLiteral("hint"));
            text->addWidget(detailLabel);
        }
        layout->addLayout(text, 1);
        list->addWidget(row);
        return layout;
    }

    QLabel *makeEmpty(const QString &text, QWidget *parent)
    {
        auto *label = new QLabel(text, parent);
        label->setObjectName(QStringLiteral("hint"));
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        label->setContentsMargins(0, 24, 0, 24);
        return label;
    }
}

LibraryTab::LibraryTab(MetronomeEngine *engine, PracticeSession *session, LibraryStore *library, QWidget *parent)
    : QWidget(parent), engine_(engine), session_(session), library_(library)
{
    auto *page = new QVBoxLayout(this);
    page->setContentsMargins(20, 20, 20, 20);
    page->setSpacing(14);

    sections_ = new ChipRow({txt("Preajustes", "Presets", "Préréglages"), txt("Repertorios", "Setlists", "Répertoires")}, this);
    sections_->setCurrent(0);
    page->addWidget(sections_);

    pages_ = new QStackedWidget(this);
    page->addWidget(pages_);
    page->addStretch();

    // Preajustes
    auto *presetsPage = new QWidget(pages_);
    auto *presetsColumn = new QVBoxLayout(presetsPage);
    presetsColumn->setContentsMargins(0, 0, 0, 0);
    presetsColumn->setSpacing(10);
    auto *save = makeButton(txt("Guardar el actual", "Save current", "Enregistrer l'actuel"), "primary", presetsPage);
    save->setToolTip(QStringLiteral("Ctrl+S"));
    auto *saveRow = new QHBoxLayout;
    auto *importButton = makeButton(txt("Importar…", "Import…", "Importer…"), "small", presetsPage);
    auto *exportButton = makeButton(txt("Exportar…", "Export…", "Exporter…"), "small", presetsPage);
    importButton->setToolTip(txt("Añade preajustes y repertorios desde un archivo", "Adds presets and setlists from a file", "Ajoute des préréglages et répertoires depuis un fichier"));
    exportButton->setToolTip(txt("Guarda todos los preajustes y repertorios en un archivo", "Saves all presets and setlists to a file", "Enregistre tous les préréglages et répertoires dans un fichier"));
    saveRow->addWidget(save);
    saveRow->addStretch();
    saveRow->addWidget(importButton);
    saveRow->addWidget(exportButton);
    presetsColumn->addLayout(saveRow);
    undoBar_ = new QFrame(presetsPage);
    undoBar_->setObjectName(QStringLiteral("banner"));
    auto *undoRow = new QHBoxLayout(undoBar_);
    undoRow->setContentsMargins(14, 6, 8, 6);
    undoText_ = new QLabel(undoBar_);
    undoText_->setObjectName(QStringLiteral("bannerText"));
    auto *undo = makeButton(txt("Deshacer", "Undo", "Annuler"), "small", undoBar_);
    undoRow->addWidget(undoText_, 1);
    undoRow->addWidget(undo);
    undoBar_->hide();
    presetsColumn->addWidget(undoBar_);
    auto *presetHost = new QWidget(presetsPage);
    presetList_ = new QVBoxLayout(presetHost);
    presetList_->setContentsMargins(0, 0, 0, 0);
    presetList_->setSpacing(8);
    presetsColumn->addWidget(presetHost);
    pages_->addWidget(presetsPage);

    // Repertorios
    auto *setlistsPage = new QWidget(pages_);
    auto *setlistsColumn = new QVBoxLayout(setlistsPage);
    setlistsColumn->setContentsMargins(0, 0, 0, 0);
    setlistsColumn->setSpacing(10);
    auto *create = makeButton(txt("Nuevo repertorio", "New setlist", "Nouveau répertoire"), "primary", setlistsPage);
    auto *createRow = new QHBoxLayout;
    createRow->addWidget(create);
    createRow->addStretch();
    setlistsColumn->addLayout(createRow);
    auto *setlistHost = new QWidget(setlistsPage);
    setlistList_ = new QVBoxLayout(setlistHost);
    setlistList_->setContentsMargins(0, 0, 0, 0);
    setlistList_->setSpacing(8);
    setlistsColumn->addWidget(setlistHost);
    pages_->addWidget(setlistsPage);

    // Detalle de un repertorio
    auto *detailPage = new QWidget(pages_);
    detailBody_ = new QVBoxLayout(detailPage);
    detailBody_->setContentsMargins(0, 0, 0, 0);
    detailBody_->setSpacing(10);
    pages_->addWidget(detailPage);

    undoTimer_ = new QTimer(this);
    undoTimer_->setSingleShot(true);
    undoTimer_->setInterval(kUndoMs);
    connect(undoTimer_, &QTimer::timeout, this, [this]()
            {
                undoBar_->hide();
                undoPreset_.reset(); });
    connect(undo, &QPushButton::clicked, this, [this]()
            {
                if (undoPreset_)
                    library_->restorePreset(*undoPreset_, undoIndex_);
                undoPreset_.reset();
                undoBar_->hide();
                undoTimer_->stop(); });

    connect(sections_, &ChipRow::activated, this, [this](int i)
            { pages_->setCurrentIndex(i); });
    connect(save, &QPushButton::clicked, this, &LibraryTab::saveCurrentAsPreset);
    connect(importButton, &QPushButton::clicked, this, &LibraryTab::importFile);
    connect(exportButton, &QPushButton::clicked, this, &LibraryTab::exportFile);
    connect(create, &QPushButton::clicked, this, [this]()
            {
                const QString name = askName(txt("Nuevo repertorio", "New setlist", "Nouveau répertoire"), {});
                if (!name.isEmpty())
                    openSetlist(library_->createSetlist(name).id); });
    connect(library_, &LibraryStore::presetsChanged, this, [this]()
            {
                rebuildPresets();
                rebuildDetail(); });
    connect(library_, &LibraryStore::setlistsChanged, this, [this]()
            {
                rebuildSetlists();
                rebuildDetail(); });

    rebuildPresets();
    rebuildSetlists();
}

QString LibraryTab::askName(const QString &title, const QString &initial)
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, title, txt("Nombre", "Name", "Nom"), QLineEdit::Normal, initial, &ok).trimmed();
    return ok ? name : QString();
}

QString LibraryTab::summary(const Preset &p) const
{
    const ClickConfig &c = p.config;
    return QStringLiteral("%1 BPM · %2/%3 · %4 · %5")
        .arg(c.bpm)
        .arg(c.beats)
        .arg(c.unit)
        .arg(subdivisionLabel(c.subdivision), soundLabel(c.sound));
}

void LibraryTab::saveCurrentAsPreset()
{
    const QString name = askName(txt("Guardar como preajuste", "Save as preset", "Enregistrer comme préréglage"), {});
    if (name.isEmpty())
        return;
    library_->savePreset(name, engine_->config());
    sections_->setCurrent(0);
    pages_->setCurrentIndex(PresetsPage);
}

void LibraryTab::deletePreset(const Preset &preset)
{
    undoPreset_ = preset;
    undoIndex_ = library_->deletePreset(preset.id);
    undoText_->setText(txt("Preajuste eliminado", "Preset deleted", "Préréglage supprimé"));
    undoBar_->show();
    undoTimer_->start();
}

void LibraryTab::rebuildPresets()
{
    clearLayout(presetList_);
    QWidget *host = presetList_->parentWidget();
    if (library_->presets().isEmpty())
    {
        presetList_->addWidget(makeEmpty(txt("Aún no hay preajustes.\nAjusta el metrónomo y guárdalo para volver a él con un clic.",
                                             "No presets yet.\nSet up the metronome and save it to come back to it with one click.",
                                             "Aucun préréglage pour l'instant.\nRéglez le métronome et enregistrez-le pour y revenir en un clic."),
                                         host));
        return;
    }
    for (const Preset &preset : library_->presets())
    {
        QHBoxLayout *row = makeRow(presetList_, host, preset.name, summary(preset));
        auto *load = makeButton(txt("Cargar", "Load", "Charger"), "small", host);
        auto *more = makeButton(QStringLiteral("⋯"), "small", host);
        more->setToolTip(txt("Más opciones", "More options", "Plus d'options"));
        row->addWidget(load);
        row->addWidget(more);

        const QString id = preset.id;
        connect(load, &QPushButton::clicked, this, [this, id]()
                {
                    if (const Preset *p = library_->presetById(id))
                        engine_->setConfig(p->config); });
        connect(more, &QPushButton::clicked, this, [this, id, more]()
                {
                    const Preset *p = library_->presetById(id);
                    if (!p)
                        return;
                    QMenu menu(this);
                    QAction *rename = menu.addAction(txt("Renombrar", "Rename", "Renommer"));
                    QAction *update = menu.addAction(txt("Actualizar con el estado actual", "Update with the current state", "Mettre à jour avec l'état actuel"));
                    QAction *remove = menu.addAction(txt("Eliminar", "Delete", "Supprimer"));
                    QAction *chosen = menu.exec(more->mapToGlobal(QPoint(0, more->height())));
                    if (chosen == rename)
                    {
                        const QString name = askName(txt("Renombrar", "Rename", "Renommer"), p->name);
                        if (!name.isEmpty())
                            library_->renamePreset(id, name);
                    }
                    else if (chosen == update)
                    {
                        library_->overwritePreset(id, engine_->config());
                    }
                    else if (chosen == remove)
                    {
                        deletePreset(*p);
                    } });
    }
}

void LibraryTab::rebuildSetlists()
{
    clearLayout(setlistList_);
    QWidget *host = setlistList_->parentWidget();
    if (library_->setlists().isEmpty())
    {
        setlistList_->addWidget(makeEmpty(txt("Aún no hay repertorios.\nUn repertorio encadena preajustes con un número de compases cada uno.",
                                              "No setlists yet.\nA setlist chains presets with a number of bars each.",
                                              "Aucun répertoire pour l'instant.\nUn répertoire enchaîne des préréglages avec un nombre de mesures chacun."),
                                          host));
        return;
    }
    for (const Setlist &setlist : library_->setlists())
    {
        const int n = static_cast<int>(setlist.items.size());
        QHBoxLayout *row = makeRow(setlistList_, host, setlist.name,
                                   n == 1 ? txt("1 pieza", "1 piece", "1 pièce") : txt("%1 piezas", "%1 pieces", "%1 pièces").arg(n));
        auto *open = makeButton(txt("Abrir", "Open", "Ouvrir"), "small", host);
        row->addWidget(open);
        const QString id = setlist.id;
        connect(open, &QPushButton::clicked, this, [this, id]()
                { openSetlist(id); });
    }
}

void LibraryTab::openSetlist(const QString &id)
{
    openSetlistId_ = id;
    rebuildDetail();
    pages_->setCurrentIndex(DetailPage);
    sections_->setCurrent(1);
}

void LibraryTab::rebuildDetail()
{
    const Setlist *setlist = library_->setlistById(openSetlistId_);
    if (!setlist)
    {
        if (pages_->currentIndex() == DetailPage)
            pages_->setCurrentIndex(SetlistsPage);
        return;
    }
    clearLayout(detailBody_);
    QWidget *host = detailBody_->parentWidget();
    const QString id = setlist->id;

    auto *header = new QHBoxLayout;
    auto *back = makeButton(txt("← Volver", "← Back", "← Retour"), "small", host);
    auto *title = new QLabel(setlist->name, host);
    title->setObjectName(QStringLiteral("strong"));
    auto *rename = makeButton(txt("Renombrar", "Rename", "Renommer"), "small", host);
    auto *remove = makeButton(txt("Eliminar", "Delete", "Supprimer"), "small", host);
    header->addWidget(back);
    header->addWidget(title, 1);
    header->addWidget(rename);
    header->addWidget(remove);
    detailBody_->addLayout(header);

    auto *hint = new QLabel(txt("Cada pieza suena los compases indicados y pasa sola a la siguiente, con su propia cuenta previa.",
                                "Each piece plays the given bars and moves on to the next by itself, with its own count-in.",
                                "Chaque pièce joue les mesures indiquées et passe seule à la suivante, avec son propre décompte."),
                            host);
    hint->setObjectName(QStringLiteral("hint"));
    hint->setWordWrap(true);
    detailBody_->addWidget(hint);

    const QList<SetlistItem> items = setlist->items;
    const auto change = [this, id, items](auto transform)
    {
        QList<SetlistItem> next = items;
        transform(next);
        library_->setSetlistItems(id, next);
    };

    auto *list = new QVBoxLayout;
    list->setSpacing(8);
    for (int i = 0; i < items.size(); ++i)
    {
        const Preset *preset = library_->presetById(items[i].presetId);
        auto *card = new QFrame(host);
        card->setObjectName(QStringLiteral("row"));
        auto *column = new QVBoxLayout(card);
        column->setContentsMargins(14, 10, 10, 10);
        column->setSpacing(6);
        auto *top = new QHBoxLayout;
        auto *text = new QVBoxLayout;
        text->setSpacing(2);
        auto *name = new QLabel(QStringLiteral("%1. %2").arg(i + 1).arg(preset ? preset->name : QStringLiteral("?")), card);
        name->setObjectName(QStringLiteral("strong"));
        text->addWidget(name);
        if (preset)
        {
            auto *detail = new QLabel(summary(*preset), card);
            detail->setObjectName(QStringLiteral("hint"));
            text->addWidget(detail);
        }
        auto *up = makeButton(QStringLiteral("▲"), "small", card);
        auto *down = makeButton(QStringLiteral("▼"), "small", card);
        auto *drop = makeButton(QStringLiteral("✕"), "small", card);
        up->setEnabled(i > 0);
        down->setEnabled(i < items.size() - 1);
        up->setToolTip(txt("Subir", "Move up", "Monter"));
        down->setToolTip(txt("Bajar", "Move down", "Descendre"));
        drop->setToolTip(txt("Quitar", "Remove", "Retirer"));
        top->addLayout(text, 1);
        top->addWidget(up);
        top->addWidget(down);
        top->addWidget(drop);
        column->addLayout(top);
        auto *bars = new StepperRow(txt("Compases", "Bars", "Mesures"), card);
        bars->setText(QString::number(items[i].bars));
        column->addWidget(bars);
        list->addWidget(card);

        connect(up, &QPushButton::clicked, this, [change, i]()
                { change([i](QList<SetlistItem> &l)
                         { l.move(i, i - 1); }); });
        connect(down, &QPushButton::clicked, this, [change, i]()
                { change([i](QList<SetlistItem> &l)
                         { l.move(i, i + 1); }); });
        connect(drop, &QPushButton::clicked, this, [change, i]()
                { change([i](QList<SetlistItem> &l)
                         { l.removeAt(i); }); });
        connect(bars, &StepperRow::stepped, this, [change, i](int d)
                { change([i, d](QList<SetlistItem> &l)
                         { l[i].bars = std::clamp(l[i].bars + d, 1, 999); }); });
    }
    detailBody_->addLayout(list);

    auto *add = makeButton(txt("+ Añadir preajuste", "+ Add preset", "+ Ajouter un préréglage"), "secondary", host);
    add->setEnabled(!library_->presets().isEmpty());
    auto *addRow = new QHBoxLayout;
    addRow->addWidget(add);
    addRow->addStretch();
    detailBody_->addLayout(addRow);
    if (library_->presets().isEmpty())
    {
        auto *need = new QLabel(txt("Primero guarda algún preajuste.", "Save a preset first.", "Enregistrez d'abord un préréglage."), host);
        need->setObjectName(QStringLiteral("hint"));
        detailBody_->addWidget(need);
    }

    auto *loop = new QCheckBox(txt("Repetir el repertorio", "Repeat the setlist", "Répéter le répertoire"), host);
    loop->setChecked(loop_);
    detailBody_->addWidget(loop);

    bool playable = false;
    for (const SetlistItem &item : items)
        playable = playable || library_->presetById(item.presetId);
    auto *play = makeButton(txt("▶  Tocar repertorio", "▶  Play setlist", "▶  Jouer le répertoire"), "primary", host);
    play->setEnabled(playable);
    detailBody_->addWidget(play);

    connect(back, &QPushButton::clicked, this, [this]()
            {
                pages_->setCurrentIndex(SetlistsPage);
                openSetlistId_.clear(); });
    connect(rename, &QPushButton::clicked, this, [this, id]()
            {
                const Setlist *s = library_->setlistById(id);
                const QString name = askName(txt("Renombrar", "Rename", "Renommer"), s ? s->name : QString());
                if (!name.isEmpty())
                    library_->renameSetlist(id, name); });
    connect(remove, &QPushButton::clicked, this, [this, id]()
            {
                const Setlist *s = library_->setlistById(id);
                if (s && confirmDialog(this, txt("¿Eliminar «%1»?", "Delete \"%1\"?", "Supprimer « %1 » ?").arg(s->name)))
                {
                    pages_->setCurrentIndex(SetlistsPage);
                    openSetlistId_.clear();
                    library_->deleteSetlist(id);
                } });
    connect(loop, &QCheckBox::toggled, this, [this](bool on)
            { loop_ = on; });
    connect(add, &QPushButton::clicked, this, [this, add, change]()
            {
                QMenu menu(this);
                for (const Preset &p : library_->presets())
                    menu.addAction(QStringLiteral("%1 — %2").arg(p.name, summary(p)))->setData(p.id);
                if (QAction *chosen = menu.exec(add->mapToGlobal(QPoint(0, add->height()))))
                {
                    const QString presetId = chosen->data().toString();
                    change([presetId](QList<SetlistItem> &l)
                           { l.append({presetId, 8}); });
                } });
    connect(play, &QPushButton::clicked, this, [this, id]()
            {
                if (const Setlist *s = library_->setlistById(id))
                    playSetlist(*s); });
}

void LibraryTab::playSetlist(const Setlist &setlist)
{
    QStringList names;
    QList<SequenceStep> steps;
    for (const SetlistItem &item : setlist.items)
    {
        if (const Preset *p = library_->presetById(item.presetId))
        {
            names << p->name;
            steps.append({p->config, item.bars});
        }
    }
    session_->stop();
    engine_->startSetlist(setlist.name, names, steps, loop_);
}

void LibraryTab::exportFile()
{
    if (library_->presets().isEmpty() && library_->setlists().isEmpty())
    {
        QMessageBox::information(this, QStringLiteral("Metrodoro"), txt("No hay nada que exportar.", "There is nothing to export.", "Il n'y a rien à exporter."));
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, txt("Exportar biblioteca", "Export library", "Exporter la bibliothèque"),
                                                QStringLiteral("metrodoro-biblioteca.json"), txt("Archivos JSON (*.json)", "JSON files (*.json)", "Fichiers JSON (*.json)"));
    if (path.isEmpty())
        return;
    if (!path.endsWith(QLatin1String(".json"), Qt::CaseInsensitive))
        path += QStringLiteral(".json");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(QJsonDocument(library_->exportJson()).toJson()) < 0)
    {
        QMessageBox::warning(this, QStringLiteral("Metrodoro"), txt("No se pudo guardar el archivo.", "The file could not be saved.", "Le fichier n'a pas pu être enregistré."));
        return;
    }
}

void LibraryTab::importFile()
{
    const QString path = QFileDialog::getOpenFileName(this, txt("Importar biblioteca", "Import library", "Importer la bibliothèque"), {},
                                                      txt("Archivos JSON (*.json)", "JSON files (*.json)", "Fichiers JSON (*.json)"));
    if (path.isEmpty())
        return;
    QFile file(path);
    QJsonParseError error;
    const QJsonDocument document = file.open(QIODevice::ReadOnly) ? QJsonDocument::fromJson(file.readAll(), &error) : QJsonDocument();
    const auto result = document.isObject() ? library_->importJson(document.object()) : LibraryStore::ImportResult{};
    if (!result.valid)
    {
        QMessageBox::warning(this, QStringLiteral("Metrodoro"),
                             txt("El archivo no es una biblioteca de Metrodoro.", "The file is not a Metrodoro library.", "Le fichier n'est pas une bibliothèque Metrodoro."));
        return;
    }
    QMessageBox::information(this, QStringLiteral("Metrodoro"),
                             txt("Se importaron %1 preajustes y %2 repertorios.", "Imported %1 presets and %2 setlists.", "%1 préréglages et %2 répertoires importés.")
                                 .arg(result.presets)
                                 .arg(result.setlists));
}
