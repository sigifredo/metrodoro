// own
#include <BeatDotsWidget.hpp>

// qt
#include <QPainter>

namespace
{
    constexpr int kDiameter = 14;
    constexpr int kGap = 10;
}

BeatDotsWidget::BeatDotsWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(kDiameter + 6);
}

void BeatDotsWidget::setBeats(int count)
{
    count_ = count;
    update();
}

void BeatDotsWidget::setCurrent(int index)
{
    current_ = index;
    update();
}

QSize BeatDotsWidget::sizeHint() const
{
    return {count_ * (kDiameter + kGap), kDiameter + 6};
}

void BeatDotsWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int total = count_ * kDiameter + (count_ - 1) * kGap;
    int x = (width() - total) / 2;
    const int y = (height() - kDiameter) / 2;

    for (int i = 0; i < count_; ++i, x += kDiameter + kGap)
    {
        const QRectF dot(x + 1, y + 1, kDiameter - 2, kDiameter - 2);
        if (i == current_)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(palette().text());
        }
        else
        {
            painter.setPen(QPen(palette().text().color(), 1.5));
            painter.setBrush(Qt::NoBrush);
        }
        painter.drawEllipse(dot);
    }
}
