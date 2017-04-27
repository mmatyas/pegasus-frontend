#include "Api.h"
#include "FrontendLayer.h"
#include "Model.h"
#include "ProcessLauncher.h"

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
    ProcessLauncher launcher;

    // the following communication is required because process handling
    // and destroying/rebuilding the frontend stack are asynchronous tasks;
    // see the relevant classes

    QObject::connect(&api, &ApiObject::prepareLaunch,
                     &frontend, &FrontendLayer::teardown);

    QObject::connect(&frontend, &FrontendLayer::teardownComplete,
                     &api, &ApiObject::onReadyToLaunch);

    QObject::connect(&api, &ApiObject::executeLaunch,
                     &launcher, &ProcessLauncher::launchGame);

    QObject::connect(&launcher, &ProcessLauncher::processFinished,
                     &api, &ApiObject::onGameFinished);

    QObject::connect(&api, &ApiObject::restoreAfterGame,
                     &frontend, &FrontendLayer::rebuild);

    return app.exec();
}
