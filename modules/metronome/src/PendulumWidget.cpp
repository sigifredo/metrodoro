// own
#include <PendulumWidget.hpp>

// qt
#include <QPainter>

// std
#include <algorithm>

namespace
{
    constexpr qreal kMaxAngle = 26.0;
}

PendulumWidget::PendulumWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(110);
    animation_.setEasingCurve(QEasingCurve::Linear);
    connect(&animation_, &QVariantAnimation::valueChanged, this, [this](const QVariant &value)
            {
                angle_ = value.toReal();
                update();
            });
}

void PendulumWidget::animateTo(qreal target, int ms)
{
    animation_.stop();
    animation_.setStartValue(angle_);
    animation_.setEndValue(target);
    animation_.setDuration(ms);
    animation_.start();
}

void PendulumWidget::setRunning(bool running)
{
    if (!running)
        animateTo(0.0, 250);
}

void PendulumWidget::swing(qint64 seq, int beatMs)
{
    animateTo(seq % 2 == 0 ? -kMaxAngle : kMaxAngle, std::max(beatMs, 120));
}

void PendulumWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QPointF pivot(width() / 2.0, height() - 6.0);
    const qreal length = height() - 16.0;
    painter.translate(pivot);
    painter.rotate(angle_);

    painter.setPen(QPen(QColor(0x88, 0x92, 0x99), 4, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QPointF(0, 0), QPointF(0, -length));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0x21, 0x96, 0xF3));
    painter.drawEllipse(QPointF(0, -length * 0.72), 11, 11);

    painter.resetTransform();
    painter.setBrush(QColor(0x88, 0x92, 0x99));
    painter.drawEllipse(pivot, 6, 6);
}
