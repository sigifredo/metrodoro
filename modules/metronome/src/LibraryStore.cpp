// own
#include <LibraryStore.hpp>

// qt
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>

// std
#include <algorithm>

namespace
{
    QString newId()
    {
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    QJsonArray readArray(QSettings &settings, const char *key)
    {
        return QJsonDocument::fromJson(settings.value(key).toByteArray()).array();
    }
}

LibraryStore::LibraryStore(QObject *parent)
    : QObject(parent)
{
    settings_.beginGroup(QStringLiteral("library"));
    load();
}

void LibraryStore::load()
{
    for (const QJsonValue &v : readArray(settings_, "presets"))
    {
        const QJsonObject o = v.toObject();
        if (o.value("id").toString().isEmpty())
            continue;
        presets_.append({o.value("id").toString(), o.value("name").toString(), ClickConfig::fromJson(o.value("config").toObject())});
    }
    for (const QJsonValue &v : readArray(settings_, "setlists"))
    {
        const QJsonObject o = v.toObject();
        if (o.value("id").toString().isEmpty())
            continue;
        Setlist s{o.value("id").toString(), o.value("name").toString(), {}};
        for (const QJsonValue &item : o.value("items").toArray())
            s.items.append({item.toObject().value("preset").toString(), std::clamp(item.toObject().value("bars").toInt(8), 1, 999)});
        setlists_.append(s);
    }
}

void LibraryStore::storePresets()
{
    QJsonArray array;
    for (const Preset &p : presets_)
        array.append(QJsonObject{{"id", p.id}, {"name", p.name}, {"config", p.config.toJson()}});
    settings_.setValue("presets", QJsonDocument(array).toJson(QJsonDocument::Compact));
    emit presetsChanged();
}

void LibraryStore::storeSetlists()
{
    QJsonArray array;
    for (const Setlist &s : setlists_)
    {
        QJsonArray items;
        for (const SetlistItem &i : s.items)
            items.append(QJsonObject{{"preset", i.presetId}, {"bars", i.bars}});
        array.append(QJsonObject{{"id", s.id}, {"name", s.name}, {"items", items}});
    }
    settings_.setValue("setlists", QJsonDocument(array).toJson(QJsonDocument::Compact));
    emit setlistsChanged();
}

const Preset *LibraryStore::presetById(const QString &id) const
{
    for (const Preset &p : presets_)
    {
        if (p.id == id)
            return &p;
    }
    return nullptr;
}

const Setlist *LibraryStore::setlistById(const QString &id) const
{
    for (const Setlist &s : setlists_)
    {
        if (s.id == id)
            return &s;
    }
    return nullptr;
}

Preset LibraryStore::savePreset(const QString &name, const ClickConfig &config)
{
    const Preset p{newId(), name.trimmed(), config};
    presets_.append(p);
    storePresets();
    return p;
}

void LibraryStore::renamePreset(const QString &id, const QString &name)
{
    for (Preset &p : presets_)
    {
        if (p.id == id)
            p.name = name.trimmed();
    }
    storePresets();
}

void LibraryStore::overwritePreset(const QString &id, const ClickConfig &config)
{
    for (Preset &p : presets_)
    {
        if (p.id == id)
            p.config = config;
    }
    storePresets();
}

int LibraryStore::deletePreset(const QString &id)
{
    int index = -1;
    for (int i = 0; i < presets_.size(); ++i)
    {
        if (presets_[i].id == id)
            index = i;
    }
    if (index < 0)
        return -1;
    presets_.removeAt(index);
    storePresets();

    for (Setlist &s : setlists_)
        s.items.removeIf([&id](const SetlistItem &i)
                         { return i.presetId == id; });
    storeSetlists();
    return index;
}

void LibraryStore::restorePreset(const Preset &preset, int index)
{
    presets_.insert(std::clamp(index, 0, static_cast<int>(presets_.size())), preset);
    storePresets();
}

Setlist LibraryStore::createSetlist(const QString &name)
{
    const Setlist s{newId(), name.trimmed(), {}};
    setlists_.append(s);
    storeSetlists();
    return s;
}

void LibraryStore::renameSetlist(const QString &id, const QString &name)
{
    for (Setlist &s : setlists_)
    {
        if (s.id == id)
            s.name = name.trimmed();
    }
    storeSetlists();
}

void LibraryStore::setSetlistItems(const QString &id, const QList<SetlistItem> &items)
{
    for (Setlist &s : setlists_)
    {
        if (s.id == id)
            s.items = items;
    }
    storeSetlists();
}

void LibraryStore::deleteSetlist(const QString &id)
{
    setlists_.removeIf([&id](const Setlist &s)
                       { return s.id == id; });
    storeSetlists();
}
