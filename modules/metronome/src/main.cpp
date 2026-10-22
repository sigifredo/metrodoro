// own
#include <MainWindow.hpp>

// qt
#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QStyleFactory>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("Metronomo");
    QApplication::setApplicationName("metronome");
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/metronome/icons/metronome.svg")));
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    QFile styleFile(QStringLiteral(":/metronome/style/style.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));

    MainWindow window;
    window.resize(480, 760);
    window.show();

    return app.exec();
}
