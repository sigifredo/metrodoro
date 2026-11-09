#ifndef SUBDIVISIONSELECTOR_HPP
#define SUBDIVISIONSELECTOR_HPP

// own
#include <ClickConfig.hpp>

// qt
#include <QList>
#include <QWidget>

class SubdivisionTile;

/// Las nueve subdivisiones como fichas: nota (o puntos de huecos) y nombre.
class SubdivisionSelector : public QWidget
{
    Q_OBJECT

public:
    explicit SubdivisionSelector(QWidget *parent = nullptr);

    void setSelection(Subdivision subdivision);

signals:
    void selected(Subdivision subdivision);

private:
    QList<SubdivisionTile *> tiles_;
};

#endif // SUBDIVISIONSELECTOR_HPP
