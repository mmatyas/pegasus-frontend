#include "Api.h"
#include "FrontendLayer.h"
#include "Model.h"

#include <QGuiApplication>
#include <QQmlContext>


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<Model::Platform>("PegasusAPI", 0, 1, "Platform");
    qmlRegisterUncreatableType<Model::Game>("PegasusAPI", 0, 1, "Game",
        QObject::tr("You cannot create this type in QML.")); // TODO: make this change for the rest
    qmlRegisterType<Model::GameAssets>("PegasusAPI", 0, 1, "GameAssets");

    ApiObject api;
    FrontendLayer frontend(&api);

    // the following communication is required because destroying the
    // frontend stack when launching a game is not synchronous;
    // see the relevant classes
    QObject::connect(&api, &ApiObject::requestLaunch,
                     &frontend, &FrontendLayer::onLaunchRequested);
    QObject::connect(&frontend, &FrontendLayer::readyToLaunch,
                     &api, &ApiObject::onReadyToLaunch);
    QObject::connect(&api, &ApiObject::executeCommand,
                     &frontend, &FrontendLayer::onExecuteCommand);

    return app.exec();
}
