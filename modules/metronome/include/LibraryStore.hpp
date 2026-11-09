#ifndef LIBRARYSTORE_HPP
#define LIBRARYSTORE_HPP

// own
#include <ClickConfig.hpp>

// qt
#include <QList>
#include <QObject>
#include <QSettings>

struct Preset
{
    QString id;
    QString name;
    ClickConfig config;
};

/// Entrada de un repertorio: un preajuste que suena `bars` compases.
struct SetlistItem
{
    QString presetId;
    int bars = 8;
};

struct Setlist
{
    QString id;
    QString name;
    QList<SetlistItem> items;
};

/// Preajustes y repertorios guardados en JSON. Borrar un preajuste lo quita también de los repertorios.
class LibraryStore : public QObject
{
    Q_OBJECT

public:
    explicit LibraryStore(QObject *parent = nullptr);

    const QList<Preset> &presets() const { return presets_; }
    const QList<Setlist> &setlists() const { return setlists_; }
    const Preset *presetById(const QString &id) const;
    const Setlist *setlistById(const QString &id) const;

    Preset savePreset(const QString &name, const ClickConfig &config);
    void renamePreset(const QString &id, const QString &name);
    void overwritePreset(const QString &id, const ClickConfig &config);
    /// Borra el preajuste y devuelve su posición para poder deshacer.
    int deletePreset(const QString &id);
    void restorePreset(const Preset &preset, int index);

    Setlist createSetlist(const QString &name);
    void renameSetlist(const QString &id, const QString &name);
    void setSetlistItems(const QString &id, const QList<SetlistItem> &items);
    void deleteSetlist(const QString &id);

signals:
    void presetsChanged();
    void setlistsChanged();

private:
    void storePresets();
    void storeSetlists();
    void load();

    QSettings settings_;
    QList<Preset> presets_;
    QList<Setlist> setlists_;
};

#endif // LIBRARYSTORE_HPP
