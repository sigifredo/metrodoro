// own
#include <MainWindow.hpp>

// qt
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QIcon>
#include <QStyleFactory>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("Metronomo");
    QApplication::setApplicationName("metronome");
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/metronome/icons/metronome.svg")));
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    // Inter para el texto y Hack (monoespaciada) para las cifras; ver resources/fonts.
    for (const char *name : {"Inter-Regular", "Inter-Medium", "Inter-SemiBold", "Hack-Regular", "Hack-Bold"})
        QFontDatabase::addApplicationFont(QStringLiteral(":/metronome/fonts/%1.ttf").arg(QLatin1String(name)));
    QApplication::setFont(QFont(QStringLiteral("Inter")));

    QFile styleFile(QStringLiteral(":/metronome/style/style.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));

    MainWindow window;
    window.resize(480, 920);
    window.show();

    return app.exec();
}
