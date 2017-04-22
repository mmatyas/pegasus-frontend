#include "Api.h"
#include "FrontendLayer.h"
#include "Model.h"

#include <QGuiApplication>
#include <QQmlContext>


int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath("lib");
    QGuiApplication app(argc, argv);


    static constexpr auto API_URI = "Pegasus.Model";
    const QString error_msg = QObject::tr("Sorry, you cannot create this type in QML.");
    qmlRegisterUncreatableType<Model::Platform>(API_URI, 0, 2, "Platform", error_msg);
    qmlRegisterUncreatableType<Model::Game>(API_URI, 0, 2, "Game", error_msg);
    qmlRegisterUncreatableType<Model::GameAssets>(API_URI, 0, 2, "GameAssets", error_msg);


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
