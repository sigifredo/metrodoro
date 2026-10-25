// own
#include <PlayButton.hpp>

// qt
#include <QPainter>
#include <QPolygonF>

PlayButton::PlayButton(QWidget *parent)
    : QAbstractButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(sizeHint());
}

void PlayButton::setPlaying(bool playing)
{
    playing_ = playing;
    update();
}

void PlayButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawEllipse(rect().adjusted(2, 2, -2, -2));

    painter.setBrush(QColor(0x21, 0x96, 0xF3));
    const QPointF c = rect().center();
    if (playing_)
    {
        painter.drawRoundedRect(QRectF(c.x() - 11, c.y() - 12, 8, 24), 1.5, 1.5);
        painter.drawRoundedRect(QRectF(c.x() + 3, c.y() - 12, 8, 24), 1.5, 1.5);
    }
    else
    {
        painter.drawPolygon(QPolygonF{{c.x() - 8, c.y() - 13}, {c.x() - 8, c.y() + 13}, {c.x() + 13, c.y()}});
    }
}
