#ifndef REPEATBUTTON_HPP
#define REPEATBUTTON_HPP

// qt
#include <QAbstractButton>

/// Botón circular con glifo de texto; usa `pressed`/`released` para la auto-repetición.
class RepeatButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit RepeatButton(const QString &glyph, QWidget *parent = nullptr);

    QSize sizeHint() const override { return {48, 48}; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    bool hovered_ = false;
};

#endif // REPEATBUTTON_HPP
