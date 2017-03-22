#include "Api.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    ApiObject pegasus_api;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("pegasus", &pegasus_api);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
