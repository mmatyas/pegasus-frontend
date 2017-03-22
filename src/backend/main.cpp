#include "Es2XmlReader.h"

#include "Model.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    Model::PlatformModel platform_model;
    Es2XmlReader::read(platform_model); // TODO: check result

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("platforms", &platform_model);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
