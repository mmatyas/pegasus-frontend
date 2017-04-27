#include "FrontendLayer.h"

#include <QQmlContext>


FrontendLayer::FrontendLayer(QObject* api, QObject* parent)
    : QObject(parent)
{
    rebuild(api);
}

void FrontendLayer::rebuild(QObject* api)
{
    Q_ASSERT(api);
    Q_ASSERT(!engine);

    engine = new QQmlApplicationEngine();
    engine->addImportPath("qml");
    engine->rootContext()->setContextProperty(QStringLiteral("pegasus"), api);
    engine->load(QUrl(QStringLiteral("qrc:/frontend/main.qml")));

    emit rebuildComplete();
}

void FrontendLayer::teardown()
{
    Q_ASSERT(engine);

    // signal forwarding
    connect(engine.data(), &QQmlApplicationEngine::destroyed,
            this, &FrontendLayer::teardownComplete);

    engine->deleteLater();
}
