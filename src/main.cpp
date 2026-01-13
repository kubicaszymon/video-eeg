#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Ustaw Basic style aby customizacja działała
    QQuickStyle::setStyle("Basic");

    QQmlApplicationEngine engine;

    const QUrl url{QStringLiteral("qrc:/qt/qml/videoEeg/qml/main.qml")};
    engine.load(url);

    if (engine.rootObjects().isEmpty())
    {
        return -1;
    }

    return app.exec();
}
