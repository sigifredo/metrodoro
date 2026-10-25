#ifndef BEATDOTSWIDGET_HPP
#define BEATDOTSWIDGET_HPP

// qt
#include <QWidget>

/// Fila de círculos (uno por tiempo del compás); el actual va relleno.
class BeatDotsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BeatDotsWidget(QWidget *parent = nullptr);

    void setBeats(int count);
    void setCurrent(int index);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int count_ = 2;
    int current_ = -1;
};

#endif // BEATDOTSWIDGET_HPP
