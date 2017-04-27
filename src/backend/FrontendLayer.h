#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlApplicationEngine>


/// Manages the dynamic reload of the frontend layer
///
/// When we launch a game, the frontend stack will be teared down to save
/// resources. However, this happens asyncronously (see QObject destructor).
/// When it's done, the relevant signal will be triggered. After the actual
/// execution is finished, the frontend layer can be rebuilt again.
///
/// Some funtions require a pointer to the API object, to connect and make
/// it accessible to the frontend.
class FrontendLayer : public QObject {
    Q_OBJECT

public:
    explicit FrontendLayer(QObject* api, QObject* parent = nullptr);

public slots:
    void rebuild(QObject* api);
    void teardown();

signals:
    void rebuildComplete();
    void teardownComplete();

private:
    QPointer<QQmlApplicationEngine> engine;
};
