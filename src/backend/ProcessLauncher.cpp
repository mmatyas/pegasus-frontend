// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#include "ProcessLauncher.h"

#include "LocaleUtils.h"
#include "ScriptRunner.h"
#include "types/Collection.h"

#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>


namespace {
static constexpr auto SEPARATOR = "----------------------------------------";
} // namespace

ProcessLauncher::ProcessLauncher(QObject* parent)
    : QObject(parent)
    , process(nullptr)
{}

void ProcessLauncher::launchGame(const Types::Collection* collection, const Types::Game* game)
{
    Q_ASSERT(game);
    // collection can be null!

    // first, check the game's custom launch command
    QString launch_cmd = game->m_launch_cmd;
    // then the lauching collection's
    if (launch_cmd.isEmpty() && collection)
        launch_cmd = collection->launchCmd();
    // then the primary collection's
    if (launch_cmd.isEmpty() && collection != game->parent()) {
        auto main_coll = qobject_cast<const Types::Collection*>(game->parent());
        if (main_coll)
            launch_cmd = collection->launchCmd();
    }
    if (!launch_cmd.isEmpty())
        prepareLaunchCommand(launch_cmd, *game);

    if (launch_cmd.isEmpty()) {
        qInfo().noquote()
            << tr_log("Cannot launch the game `%1` because there is no launch command defined for it!")
               .arg(game->m_title);
    }
    else {
        beforeRun();
        runProcess(launch_cmd);
        afterRun();
    }

    // TODO: report error to the caller
    emit processFinished();
}

void ProcessLauncher::prepareLaunchCommand(QString& launch_cmd, const Types::Game& game) const
{
    launch_cmd
        .replace(QLatin1String("{file.path}"), game.m_fileinfo.absoluteFilePath())
        .replace(QLatin1String("{file.name}"), game.m_fileinfo.fileName())
        .replace(QLatin1String("{file.basename}"), game.m_fileinfo.completeBaseName())
        .replace(QLatin1String("{file.dir}"), game.m_fileinfo.absolutePath());
}

void ProcessLauncher::runProcess(const QString& command)
{
    qInfo().noquote() << tr_log("Executing command: `%1`").arg(command);

    Q_ASSERT(!process);
    process = new QProcess();

    // set up signals and slots
    connect(process.data(), &QProcess::started, this, &ProcessLauncher::onProcessStarted);
    connect(process.data(), &QProcess::errorOccurred, this, &ProcessLauncher::onProcessFailed);
    connect(process.data(), static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &ProcessLauncher::onProcessFinished);

    // run the command
    process->setProcessChannelMode(QProcess::ForwardedChannels);
    process->start(command, QProcess::ReadOnly);

    // wait
    process->waitForFinished(-1);
    process->terminate(); // TODO: `terminate` is probably unnecessary
    process->deleteLater();
}

void ProcessLauncher::onProcessStarted()
{
    Q_ASSERT(process);
    qInfo().noquote() << tr_log("Process %1 started").arg(process->processId());
    qInfo().noquote() << SEPARATOR;
}

void ProcessLauncher::onProcessFailed(QProcess::ProcessError error)
{
    Q_ASSERT(process);
    switch (error) {
        case QProcess::FailedToStart:
            qWarning().noquote() << tr_log("Could not run the command `%1`; either the"
                                           " invoked program is missing, or you don't have"
                                           " the permission to run it.")
                                    .arg(process->program());
            break;
        case QProcess::Crashed:
            qWarning().noquote() << tr_log("The external program `%1` has crashed")
                                    .arg(process->program());
            break;
        case QProcess::Timedout:
            qWarning().noquote() << tr_log("The command `%1` has not started in a"
                                           " reasonable amount of time")
                                    .arg(process->program());
            break;
        case QProcess::ReadError:
        case QProcess::WriteError:
            // We don't communicate with the launched process at the moment
            Q_UNREACHABLE();
            break;
        default:
            qWarning().noquote() << tr_log("Running the command `%1` failed due to an unknown error")
                                    .arg(process->program());
            break;
    }
}

void ProcessLauncher::onProcessFinished(int exitcode, QProcess::ExitStatus exitstatus)
{
    Q_ASSERT(process);
    qInfo().noquote() << SEPARATOR;

    switch (exitstatus) {
        case QProcess::NormalExit:
            qInfo().noquote() << tr_log("The external program has finished cleanly, with exit code %2")
                                 .arg(exitcode);
            break;
        case QProcess::CrashExit:
            qInfo().noquote() << tr_log("The external program has crashed on exit, with exit code %2")
                                 .arg(exitcode);
            break;
        default:
            // If you reach this branch, there was an API change in Qt
            Q_UNREACHABLE();
            break;
    }
}

void ProcessLauncher::beforeRun() const
{
    // call the relevant scripts
    using ScriptEvent = ScriptRunner::EventType;
    ScriptRunner::findAndRunScripts(ScriptEvent::PROCESS_STARTED);
}

void ProcessLauncher::afterRun() const
{
    // call the relevant scripts
    using ScriptEvent = ScriptRunner::EventType;
    ScriptRunner::findAndRunScripts(ScriptEvent::PROCESS_FINISHED);
}
