#ifndef PLAYBUTTON_HPP
#define PLAYBUTTON_HPP

// qt
#include <QAbstractButton>

/// Círculo blanco con glifo play/pause en el acento naranja.
class PlayButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit PlayButton(QWidget *parent = nullptr);

    void setPlaying(bool playing);
    QSize sizeHint() const override { return {64, 64}; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool playing_ = false;
};

#endif // PLAYBUTTON_HPP
