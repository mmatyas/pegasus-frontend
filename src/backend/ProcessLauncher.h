#pragma once

#include "Model.h"

#include <QObject>
#include <QPointer>
#include <QProcess>


/// Launches and manages external processes
///
/// Launches external processes and detects their success or failure.
class ProcessLauncher : public QObject {
    Q_OBJECT

public:
    explicit ProcessLauncher(QObject* parent = nullptr);

signals:
    void processFinished();

public slots:
    void launchGame(const Model::Platform*, const Model::Game*);

private slots:
    void onProcessStarted();
    void onProcessFailed(QProcess::ProcessError);
    void onProcessFinished(int, QProcess::ExitStatus);

private:
    QPointer<QProcess> process;

    QString createLaunchCommand(const Model::Platform*, const Model::Game*);
    void runProcess(const QString&);
};
