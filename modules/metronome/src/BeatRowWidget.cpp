// own
#include <BeatRowWidget.hpp>
#include <Texts.hpp>

// qt
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

BeatRowWidget::BeatRowWidget(QWidget *parent)
    : QWidget(parent), accents_(defaultAccents(4, 4))
{
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void BeatRowWidget::setAccents(const QList<Accent> &accents)
{
    const bool resized = accents.size() != accents_.size();
    accents_ = accents;
    if (resized)
        updateGeometry();
    update();
}

void BeatRowWidget::setCurrent(int index, bool muted, bool countIn)
{
    current_ = index;
    muted_ = muted;
    countIn_ = countIn;
    update();
}

void BeatRowWidget::setPoly(int count, int current)
{
    const bool resized = count != polyCount_;
    polyCount_ = count;
    polyCurrent_ = current;
    if (resized)
        updateGeometry();
    update();
}

int BeatRowWidget::perRow() const
{
    const int n = static_cast<int>(accents_.size());
    return n > kMaxPerRow ? (n + 1) / 2 : std::max(n, 1);
}

QSize BeatRowWidget::sizeHint() const
{
    const int rows = (static_cast<int>(accents_.size()) + perRow() - 1) / perRow();
    return {perRow() * kCell, std::max(rows, 1) * kCell + (polyCount_ > 0 ? kPolyRow : 0)};
}

QRect BeatRowWidget::cellRect(int index) const
{
    const int per = perRow();
    const int n = static_cast<int>(accents_.size());
    const int row = index / per;
    const int inRow = std::min(per, n - row * per);
    const int x0 = (width() - inRow * kCell) / 2;
    return {x0 + (index % per) * kCell, row * kCell, kCell, kCell};
}

void BeatRowWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    for (int i = 0; i < accents_.size(); ++i)
    {
        if (cellRect(i).contains(event->position().toPoint()))
        {
            emit tapped(i);
            QToolTip::showText(event->globalPosition().toPoint(),
                               txt("Tiempo %1: %2", "Beat %1: %2", "Temps %1 : %2").arg(i + 1).arg(accentLabel(nextAccent(accents_[i]))),
                               this);
            return;
        }
    }
}

void BeatRowWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QColor idle(0x88, 0x92, 0x99);
    const QColor active = countIn_ ? QColor(0x6F, 0xCF, 0x97) : QColor(0x21, 0x96, 0xF3);
    if (muted_)
        painter.setOpacity(0.4);

    for (int i = 0; i < accents_.size(); ++i)
    {
        const Accent accent = accents_[i];
        const bool on = i == current_;
        qreal size = 16;
        switch (accent)
        {
        case Accent::Strong:
            size = 32;
            break;
        case Accent::Medium:
            size = 26;
            break;
        case Accent::Weak:
            size = 20;
            break;
        case Accent::Mute:
            size = 14;
            break;
        }
        if (on)
            size *= 1.15;

        const QPointF c = QRectF(cellRect(i)).center();
        const QRectF dot(c.x() - size / 2, c.y() - size / 2, size, size);

        QColor fill = idle;
        QColor line = idle;
        bool filled = false;
        qreal lineWidth = 2;
        if (on)
        {
            line = active;
            fill = active;
            filled = accent != Accent::Mute;
        }
        else if (accent == Accent::Strong)
        {
            fill.setAlphaF(0.7);
            filled = true;
        }
        else if (accent == Accent::Medium)
        {
            fill.setAlphaF(0.4);
            filled = true;
        }
        else if (accent == Accent::Mute)
        {
            line.setAlphaF(0.5);
            lineWidth = 1;
        }

        if (filled)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(fill);
        }
        else
        {
            painter.setPen(QPen(line, lineWidth));
            painter.setBrush(Qt::NoBrush);
        }
        painter.drawEllipse(dot.adjusted(1, 1, -1, -1));
    }

    if (polyCount_ > 0)
    {
        painter.setOpacity(1.0);
        const int rows = (static_cast<int>(accents_.size()) + perRow() - 1) / perRow();
        const qreal y = std::max(rows, 1) * kCell + kPolyRow / 2.0 - 2;
        const qreal step = std::min<qreal>(28.0, (width() - 16.0) / polyCount_);
        qreal x = (width() - step * polyCount_) / 2 + step / 2;
        for (int i = 0; i < polyCount_; ++i, x += step)
        {
            const bool on = i == polyCurrent_;
            if (on)
            {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(0xF9, 0xF9, 0xF9));
            }
            else
            {
                painter.setPen(QPen(QColor(0xF9, 0xF9, 0xF9, 150), 1.5));
                painter.setBrush(Qt::NoBrush);
            }
            painter.drawRect(QRectF(x - 5, y - 5, 10, 10));
        }
    }
}
