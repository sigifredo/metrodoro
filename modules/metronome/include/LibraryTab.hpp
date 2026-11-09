#ifndef LIBRARYTAB_HPP
#define LIBRARYTAB_HPP

// own
#include <LibraryStore.hpp>

// qt
#include <QWidget>

class ChipRow;
class MetronomeEngine;
class PracticeSession;
class QCheckBox;
class QFrame;
class QLabel;
class QStackedWidget;
class QTimer;
class QVBoxLayout;

/// Preajustes con nombre y repertorios (listas de preajustes con compases por entrada y avance automático).
class LibraryTab : public QWidget
{
    Q_OBJECT

public:
    LibraryTab(MetronomeEngine *engine, PracticeSession *session, LibraryStore *library, QWidget *parent = nullptr);

    /// Pide un nombre y guarda la configuración actual como preajuste.
    void saveCurrentAsPreset();

private:
    enum Page
    {
        PresetsPage,
        SetlistsPage,
        DetailPage
    };

    void rebuildPresets();
    void rebuildSetlists();
    void rebuildDetail();
    void openSetlist(const QString &id);
    void playSetlist(const Setlist &setlist);
    QString askName(const QString &title, const QString &initial);
    QString summary(const Preset &preset) const;
    void deletePreset(const Preset &preset);

    MetronomeEngine *engine_;
    PracticeSession *session_;
    LibraryStore *library_;

    ChipRow *sections_ = nullptr;
    QStackedWidget *pages_ = nullptr;
    QVBoxLayout *presetList_ = nullptr;
    QVBoxLayout *setlistList_ = nullptr;
    QVBoxLayout *detailBody_ = nullptr;
    QFrame *undoBar_ = nullptr;
    QTimer *undoTimer_ = nullptr;
    QLabel *undoText_ = nullptr;

    QString openSetlistId_;
    bool loop_ = false;
    std::optional<Preset> undoPreset_;
    int undoIndex_ = 0;
};

#endif // LIBRARYTAB_HPP
