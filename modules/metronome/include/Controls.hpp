#ifndef CONTROLS_HPP
#define CONTROLS_HPP

// qt
#include <QFrame>
#include <QLabel>
#include <QList>
#include <QStringList>
#include <QWidget>

class QAbstractButton;
class QCheckBox;
class QVBoxLayout;

/// Etiqueta que avisa al pulsarla (la cifra del tempo se edita así).
class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    using QLabel::QLabel;

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
};

/// Fila de opciones excluyentes tipo "chip". Reparte las filas si no caben.
class ChipRow : public QWidget
{
    Q_OBJECT

public:
    explicit ChipRow(const QStringList &labels, QWidget *parent = nullptr);

    /// Marca la opción; -1 las desmarca todas.
    void setCurrent(int index);

signals:
    void activated(int index);

private:
    QList<QAbstractButton *> buttons_;
};

/// Fila "título   −  valor  +" con botones que repiten al mantener.
class StepperRow : public QWidget
{
    Q_OBJECT

public:
    explicit StepperRow(const QString &title, QWidget *parent = nullptr);

    void setText(const QString &text);

signals:
    void stepped(int delta);

private:
    QLabel *value_ = nullptr;
};

/// Casilla con una línea de ayuda debajo.
QWidget *makeSwitchRow(QCheckBox *box, const QString &hint, QWidget *parent);

/// Tarjeta con título y un diseño vertical para su contenido.
QFrame *makeCard(const QString &title, QWidget *parent, QVBoxLayout **body);

QFrame *makeSeparator(QWidget *parent);

/// Pregunta de aceptar o cancelar con los textos de la interfaz.
bool confirmDialog(QWidget *parent, const QString &question);

/// Envuelve el contenido de una pestaña en un área con desplazamiento sin marco.
QWidget *makeScrollPage(QWidget *content, QWidget *parent);

#endif // CONTROLS_HPP
