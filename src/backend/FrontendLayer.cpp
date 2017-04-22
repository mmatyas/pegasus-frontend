#include "FrontendLayer.h"

#include "Api.h"

#include <QDebug>
#include <QProcess>
#include <QQmlContext>


FrontendLayer::FrontendLayer(ApiObject* api)
    : QObject(nullptr)
{
    rebuild(api);
}

void FrontendLayer::onLaunchRequested()
{
    // signal forwarding
    connect(engine.data(), &QQmlApplicationEngine::destroyed,
            this, &FrontendLayer::readyToLaunch);

    engine->deleteLater();
}

void FrontendLayer::onExecuteCommand(ApiObject* api, QString cmd)
{
    qInfo() << tr("Executing command: `%1`").arg(cmd);

    const int exitcode = QProcess::execute(cmd);
    switch (exitcode) {
        case -2:
            qInfo().noquote() << tr("Could not run the launch command; either the invoked program "
                                    "is missing, or you don't have the permission to run it.");
            break;
        case -1:
            qInfo().noquote() << tr("The external program has crashed");
            break;
        default:
            qInfo().noquote() << tr("The external program has finished with exit code %1")
                                 .arg(exitcode);
            break;
    }

    rebuild(api);
    emit externalFinished();
}

void FrontendLayer::rebuild(ApiObject* api)
{
    Q_ASSERT(api);
    Q_ASSERT(!engine);

    engine = new QQmlApplicationEngine();
    engine->addImportPath("qml");
    engine->rootContext()->setContextProperty(QStringLiteral("pegasus"), api);
    engine->load(QUrl(QStringLiteral("qrc:/frontend/main.qml")));
}
