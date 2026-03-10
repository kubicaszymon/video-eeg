#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtCore/qlogging.h>

static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Qt Graphs emits this warning for every NaN sentinel used as a line-break
    // in the EEG sweep-cursor gap. The NaN values are intentional (see
    // EegDataModel::GAP_VALUE) and the graph renders correctly — suppress only
    // this specific message to keep the output clean.
    if (msg.contains(QLatin1String("Ignored NaN, Inf, or -Inf value")))
        return;

    QMessageLogContext ctx(context.file, context.line, context.function, context.category);
    qt_message_output(type, ctx, msg);
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);
    QApplication app(argc, argv);

    // Set Basic style for customization to work
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
