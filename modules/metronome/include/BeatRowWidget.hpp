#ifndef BEATROWWIDGET_HPP
#define BEATROWWIDGET_HPP

// own
#include <ClickConfig.hpp>

// qt
#include <QWidget>

/// Los tiempos del compás como círculos. Se pulsan para cambiar su acento: fuerte, medio, débil, silencio.
class BeatRowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BeatRowWidget(QWidget *parent = nullptr);

    void setAccents(const QList<Accent> &accents);
    /// `muted` atenúa el compás (compás mudo); `countIn` pinta el tiempo activo en verde.
    void setCurrent(int index, bool muted, bool countIn);
    QSize sizeHint() const override;

signals:
    void tapped(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    static constexpr int kCell = 46;
    static constexpr int kMaxPerRow = 8;

    int perRow() const;
    QRect cellRect(int index) const;

    QList<Accent> accents_;
    int current_ = -1;
    bool muted_ = false;
    bool countIn_ = false;
};

#endif // BEATROWWIDGET_HPP
