#include "FrontendLayer.h"

#include "Api.h"

#include <QDebug>
#include <QQmlContext>


FrontendLayer::FrontendLayer(ApiObject* api)
    : QObject(nullptr)
    , process(nullptr)
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
    static const auto SEPARATOR = "----------------------------------------";

    qInfo().noquote() << SEPARATOR;
    qInfo().noquote() << tr("Executing command: `%1`").arg(cmd);

    Q_ASSERT(!process);
    process = new QProcess();

    // set up signals and slots
    connect(process.data(), &QProcess::started, this, &FrontendLayer::onProcessStarted);
    connect(process.data(), &QProcess::errorOccurred, this, &FrontendLayer::onProcessFailed);
    connect(process.data(), static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &FrontendLayer::onProcessFinished);

    // run the command
    process->start(cmd, QProcess::ReadOnly);
    process->waitForFinished();

    rebuild(api);

    emit externalFinished();
}

void FrontendLayer::onProcessStarted()
{
    Q_ASSERT(process);
    qInfo().noquote() << tr("Process %1 started").arg(process->processId());
}

void FrontendLayer::onProcessFailed(QProcess::ProcessError error)
{
    Q_ASSERT(process);
    switch (error) {
        case QProcess::FailedToStart:
            qWarning().noquote() << tr("Could not run the command `%1`; either the"
                                       " invoked program is missing, or you don't have"
                                       " the permission to run it.")
                                    .arg(process->program());
            break;
        case QProcess::Crashed:
            qWarning().noquote() << tr("The external program `%1` has crashed")
                                    .arg(process->program());
            break;
        case QProcess::Timedout:
            qWarning().noquote() << tr("The command `%1` has not started in a"
                                       " reasonable amount of time")
                                    .arg(process->program());
            break;
        case QProcess::ReadError:
        case QProcess::WriteError:
            // We don't communicate with the launched processes at the moment
            Q_UNREACHABLE();
            break;
        default:
            qWarning().noquote() << tr("Running the command `%1` failed due to an unknown error")
                                    .arg(process->program());
            break;
    }
}

void FrontendLayer::onProcessFinished(int exitcode, QProcess::ExitStatus exitstatus)
{
    Q_ASSERT(process);
    switch (exitstatus) {
        case QProcess::NormalExit:
            qInfo().noquote() << tr("The external program has finished cleanly, with exit code %2")
                                 .arg(exitcode);
            break;
        case QProcess::CrashExit:
            qInfo().noquote() << tr("The external program has crashed on exit, with exit code %2")
                                 .arg(exitcode);
        default:
            // If you reach this branch, there was an API change in Qt
            Q_UNREACHABLE();
            break;
    }
}

void FrontendLayer::rebuild(ApiObject* api)
{
    Q_ASSERT(api);
    Q_ASSERT(!engine);

    if (process) {
        process->terminate(); // FIXME: `terminate` is probably unnecessary
        process->deleteLater();
    }

    engine = new QQmlApplicationEngine();
    engine->addImportPath("qml");
    engine->rootContext()->setContextProperty(QStringLiteral("pegasus"), api);
    engine->load(QUrl(QStringLiteral("qrc:/frontend/main.qml")));
}
