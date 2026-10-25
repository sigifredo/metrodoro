// own
#include <RepeatButton.hpp>

// qt
#include <QPainter>

RepeatButton::RepeatButton(const QString &glyph, QWidget *parent)
    : QAbstractButton(parent)
{
    setText(glyph);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(sizeHint());
}

void RepeatButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(QColor(136, 146, 153, hovered_ && isEnabled() ? 255 : 128), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(rect().adjusted(1, 1, -1, -1));

    QFont f = font();
    f.setPixelSize(22);
    f.setWeight(QFont::Medium);
    painter.setFont(f);
    painter.setPen(palette().buttonText().color());
    painter.drawText(rect(), Qt::AlignCenter, text());
}

void RepeatButton::enterEvent(QEnterEvent *)
{
    hovered_ = true;
    update();
}

void RepeatButton::leaveEvent(QEvent *)
{
    hovered_ = false;
    update();
}
