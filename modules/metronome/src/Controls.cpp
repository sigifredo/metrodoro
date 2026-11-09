// own
#include <Controls.hpp>
#include <RepeatButton.hpp>

// qt
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>

void ClickableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->position().toPoint()))
        emit clicked();
    QLabel::mouseReleaseEvent(event);
}

ChipRow::ChipRow(const QStringList &labels, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    for (int i = 0; i < labels.size(); ++i)
    {
        auto *chip = new QPushButton(labels[i], this);
        chip->setObjectName(QStringLiteral("chip"));
        chip->setCheckable(true);
        chip->setCursor(Qt::PointingHandCursor);
        chip->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(chip);
        buttons_ << chip;
        connect(chip, &QPushButton::clicked, this, [this, i]()
                {
                    setCurrent(i);
                    emit activated(i);
                });
    }
    layout->addStretch();
}

void ChipRow::setCurrent(int index)
{
    for (int i = 0; i < buttons_.size(); ++i)
        buttons_[i]->setChecked(i == index);
}

StepperRow::StepperRow(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    auto *label = new QLabel(title, this);
    value_ = new QLabel(this);
    value_->setObjectName(QStringLiteral("counter"));
    value_->setAlignment(Qt::AlignCenter);
    value_->setFixedWidth(110);
    auto *minus = new RepeatButton(QStringLiteral("−"), this);
    auto *plus = new RepeatButton(QStringLiteral("+"), this);
    minus->setFixedSize(32, 32);
    plus->setFixedSize(32, 32);

    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->addWidget(label);
    row->addStretch();
    row->addWidget(minus);
    row->addWidget(value_);
    row->addWidget(plus);

    connect(minus, &QAbstractButton::clicked, this, [this]()
            { emit stepped(-1); });
    connect(plus, &QAbstractButton::clicked, this, [this]()
            { emit stepped(+1); });
}

void StepperRow::setText(const QString &text)
{
    value_->setText(text);
}

QWidget *makeSwitchRow(QCheckBox *box, const QString &hint, QWidget *parent)
{
    auto *row = new QWidget(parent);
    auto *column = new QVBoxLayout(row);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(2);
    column->addWidget(box);
    auto *hintLabel = new QLabel(hint, row);
    hintLabel->setObjectName(QStringLiteral("hint"));
    hintLabel->setWordWrap(true);
    hintLabel->setContentsMargins(22, 0, 0, 0);
    column->addWidget(hintLabel);
    return row;
}

QFrame *makeCard(const QString &title, QWidget *parent, QVBoxLayout **body)
{
    auto *card = new QFrame(parent);
    card->setObjectName(QStringLiteral("card"));
    auto *column = new QVBoxLayout(card);
    column->setContentsMargins(16, 14, 16, 16);
    column->setSpacing(10);
    auto *heading = new QLabel(title.toUpper(), card);
    heading->setObjectName(QStringLiteral("cardTitle"));
    column->addWidget(heading);
    *body = column;
    return card;
}

QFrame *makeSeparator(QWidget *parent)
{
    auto *line = new QFrame(parent);
    line->setObjectName(QStringLiteral("separator"));
    line->setFrameShape(QFrame::HLine);
    return line;
}

QWidget *makeScrollPage(QWidget *content, QWidget *parent)
{
    auto *area = new QScrollArea(parent);
    area->setObjectName(QStringLiteral("page"));
    area->setFrameShape(QFrame::NoFrame);
    area->setWidgetResizable(true);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setWidget(content);
    content->setObjectName(QStringLiteral("pageContent"));
    return area;
}
