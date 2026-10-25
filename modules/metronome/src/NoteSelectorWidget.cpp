// own
#include <NoteSelectorWidget.hpp>

// qt
#include <QHBoxLayout>
#include <QIcon>
#include <QStyle>
#include <QToolButton>

NoteSelectorWidget::NoteSelectorWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->addStretch();

    quarter_ = makeButton(QStringLiteral("noires"), 1, false);
    eighth_ = makeButton(QStringLiteral("croches"), 2, false);
    sixteenth_ = makeButton(QStringLiteral("doubles"), 4, false);
    triplet_ = makeButton(QStringLiteral("triolets"), 3, false);
    shuffle_ = makeButton(QStringLiteral("shuffle"), 3, true);

    for (auto *button : {quarter_, eighth_, sixteenth_, triplet_, shuffle_})
        layout->addWidget(button);

    layout->addStretch();
    setSelection(1, false);
}

QToolButton *NoteSelectorWidget::makeButton(const QString &icon, int dec, bool shuffle)
{
    auto *button = new QToolButton(this);
    button->setObjectName(QStringLiteral("noteButton"));
    button->setIcon(QIcon(QStringLiteral(":/metronome/icons/%1.png").arg(icon)));
    button->setIconSize({40, 40});
    button->setFixedSize(48, 48);
    button->setCursor(Qt::PointingHandCursor);
    button->setFocusPolicy(Qt::NoFocus);
    button->setProperty("shuffle", shuffle);
    connect(button, &QToolButton::clicked, this, [this, dec, shuffle]()
            { emit selected(dec, shuffle); });
    return button;
}

void NoteSelectorWidget::setSelection(int dec, bool shuffle)
{
    const auto mark = [](QToolButton *button, bool on, const char *role)
    {
        button->setProperty("state", on ? role : "");
        button->style()->unpolish(button);
        button->style()->polish(button);
    };

    mark(quarter_, dec == 1, "set");
    mark(eighth_, dec == 2, "set");
    mark(sixteenth_, dec == 4, "set");
    mark(triplet_, dec == 3 && !shuffle, "set");
    mark(shuffle_, dec == 3 && shuffle, "active");
}
