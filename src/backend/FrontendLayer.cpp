#include "FrontendLayer.h"

#include "Api.h"

#include <QDebug>
#include <QQmlContext>
#include <QProcess>


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
    qInfo() << tr("Executing command: %1").arg(cmd);
    QProcess::execute(cmd);

    rebuild(api);
    emit externalFinished();
}

void FrontendLayer::rebuild(ApiObject* api)
{
    Q_ASSERT(api);
    Q_ASSERT(!engine);

    engine = new QQmlApplicationEngine();
    engine->rootContext()->setContextProperty(QStringLiteral("pegasus"), api);
    engine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
}
