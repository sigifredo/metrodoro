#ifndef NOTESELECTORWIDGET_HPP
#define NOTESELECTORWIDGET_HPP

// qt
#include <QWidget>

class QToolButton;

/// Cinco botones de subdivisión: negras, corcheas, semicorcheas, tresillo y shuffle.
class NoteSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoteSelectorWidget(QWidget *parent = nullptr);

    void setSelection(int dec, bool shuffle);

signals:
    void selected(int dec, bool shuffle);

private:
    QToolButton *makeButton(const QString &icon, int dec, bool shuffle);

    QToolButton *quarter_ = nullptr;
    QToolButton *eighth_ = nullptr;
    QToolButton *sixteenth_ = nullptr;
    QToolButton *triplet_ = nullptr;
    QToolButton *shuffle_ = nullptr;
};

#endif // NOTESELECTORWIDGET_HPP
