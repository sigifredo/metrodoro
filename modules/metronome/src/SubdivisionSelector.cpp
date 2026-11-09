// own
#include <SubdivisionSelector.hpp>
#include <Texts.hpp>

// qt
#include <QAbstractButton>
#include <QGridLayout>
#include <QPainter>
#include <QPixmap>

namespace
{
    constexpr int kColumns = 5;

    const char *iconFor(Subdivision s)
    {
        switch (s)
        {
        case Subdivision::Quarter:
            return "noires";
        case Subdivision::Eighth:
            return "croches";
        case Subdivision::Triplet:
            return "triolets";
        case Subdivision::Sixteenth:
            return "doubles";
        case Subdivision::Shuffle:
            return "shuffle";
        default:
            return nullptr;
        }
    }
}

class SubdivisionTile : public QAbstractButton
{
public:
    SubdivisionTile(Subdivision subdivision, QWidget *parent)
        : QAbstractButton(parent), subdivision_(subdivision)
    {
        setCheckable(true);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::NoFocus);
        setMinimumSize(88, 76);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setToolTip(subdivisionLabel(subdivision));
        if (const char *icon = iconFor(subdivision))
            icon_ = QPixmap(QStringLiteral(":/metronome/icons/%1.png").arg(QLatin1String(icon)));
    }

    QSize sizeHint() const override { return {96, 76}; }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        const bool on = isChecked();
        const QColor accent(0x21, 0x96, 0xF3);
        const QColor idle(0x88, 0x92, 0x99);
        const QColor tint = on ? accent : idle;

        const QRectF box = QRectF(rect()).adjusted(1, 1, -1, -1);
        painter.setPen(QPen(on ? accent : QColor(255, 255, 255, hovered_ ? 70 : 30), on ? 1.5 : 1));
        painter.setBrush(on ? QColor(0x21, 0x96, 0xF3, 38) : QColor(255, 255, 255, hovered_ ? 14 : 8));
        painter.drawRoundedRect(box, 10, 10);

        const QRectF art(box.center().x() - 20, box.top() + 8, 40, 36);
        if (!icon_.isNull())
        {
            QPixmap tinted(icon_.size());
            tinted.fill(Qt::transparent);
            QPainter tp(&tinted);
            tp.drawPixmap(0, 0, icon_);
            tp.setCompositionMode(QPainter::CompositionMode_SourceIn);
            tp.fillRect(tinted.rect(), tint);
            tp.end();
            painter.drawPixmap(art.toRect(), tinted);
        }
        else
        {
            const QList<bool> &mask = subdivisionMask(subdivision_);
            const qreal step = std::min<qreal>(11.0, 60.0 / mask.size());
            const qreal r = std::min<qreal>(step / 2 - 1, 5.0);
            qreal x = box.center().x() - step * mask.size() / 2 + step / 2;
            const qreal y = art.center().y();
            for (bool sounds : mask)
            {
                if (sounds)
                {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(tint);
                }
                else
                {
                    painter.setPen(QPen(tint, 1.5));
                    painter.setBrush(Qt::NoBrush);
                }
                painter.drawEllipse(QPointF(x, y), r, r);
                x += step;
            }
        }

        QFont f = font();
        f.setPixelSize(11);
        painter.setFont(f);
        painter.setPen(on ? QColor(0xF9, 0xF9, 0xF9) : QColor(0xD0, 0xD4, 0xD8));
        painter.drawText(QRectF(box.left() + 2, box.bottom() - 26, box.width() - 4, 22), Qt::AlignCenter | Qt::TextWordWrap,
                         subdivisionLabel(subdivision_));
    }

    void enterEvent(QEnterEvent *) override
    {
        hovered_ = true;
        update();
    }

    void leaveEvent(QEvent *) override
    {
        hovered_ = false;
        update();
    }

private:
    Subdivision subdivision_;
    QPixmap icon_;
    bool hovered_ = false;
};

SubdivisionSelector::SubdivisionSelector(QWidget *parent)
    : QWidget(parent)
{
    auto *grid = new QGridLayout(this);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(8);
    for (int i = 0; i < kSubdivisionCount; ++i)
    {
        const auto sub = static_cast<Subdivision>(i);
        auto *tile = new SubdivisionTile(sub, this);
        grid->addWidget(tile, i / kColumns, i % kColumns);
        tiles_ << tile;
        connect(tile, &QAbstractButton::clicked, this, [this, sub]()
                {
                    setSelection(sub);
                    emit selected(sub);
                });
    }
    for (int c = 0; c < kColumns; ++c)
        grid->setColumnStretch(c, 1);
}

void SubdivisionSelector::setSelection(Subdivision subdivision)
{
    for (int i = 0; i < tiles_.size(); ++i)
        static_cast<QAbstractButton *>(tiles_[i])->setChecked(i == static_cast<int>(subdivision));
}
