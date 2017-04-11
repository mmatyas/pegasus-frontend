#include "Api.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<Model::Platform>("PegasusAPI", 0, 1, "Platform");
    qmlRegisterType<Model::Game>("PegasusAPI", 0, 1, "Game");
    qmlRegisterType<Model::GameAssets>("PegasusAPI", 0, 1, "GameAssets");

    ApiObject pegasus_api;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("pegasus", &pegasus_api);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
