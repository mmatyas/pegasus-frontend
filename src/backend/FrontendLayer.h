#pragma once

#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QQmlApplicationEngine>

class ApiObject;


/// This class manages the dynamic reload of the frontend layer
///
/// When we launch a game, the frontend stack will be teared
/// down to save resources. This happens asyncronously
/// (see QObject destructor). When it's done, the API will be
/// notified, and it will send the actual command to execute.
class FrontendLayer : public QObject {
    Q_OBJECT

public:
    explicit FrontendLayer(ApiObject*);

signals:
    void readyToLaunch(); ///< finished tearing down the frontend layer
    void externalFinished(); ///< the external process has finished

public slots:
    void onLaunchRequested(); ///< start tearing down the frontend layer
    void onExecuteCommand(ApiObject*, QString); ///< start an external process

    void onProcessStarted();
    void onProcessFailed(QProcess::ProcessError);
    void onProcessFinished(int, QProcess::ExitStatus);

private:
    QPointer<QQmlApplicationEngine> engine;
    QPointer<QProcess> process;

    void rebuild(ApiObject*);
};
