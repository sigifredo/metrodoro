#ifndef PENDULUMWIDGET_HPP
#define PENDULUMWIDGET_HPP

// qt
#include <QVariantAnimation>
#include <QWidget>

/// Péndulo: cambia de lado en cada tiempo y tarda justo un tiempo en cruzar.
class PendulumWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PendulumWidget(QWidget *parent = nullptr);

    void setRunning(bool running);
    /// Un tiempo nuevo: el péndulo cruza hacia el lado que toque durante `beatMs`.
    void swing(qint64 seq, int beatMs);
    QSize sizeHint() const override { return {200, 110}; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void animateTo(qreal target, int ms);

    QVariantAnimation animation_;
    qreal angle_ = 0.0;
};

#endif // PENDULUMWIDGET_HPP
