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

#include "ScriptRunner.h"
#include "types/Collection.h"

#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>


static const auto SEPARATOR = "----------------------------------------";

ProcessLauncher::ProcessLauncher(QObject* parent)
    : QObject(parent)
    , process(nullptr)
{}

void ProcessLauncher::launchGame(const Types::Collection* collection, const Types::Game* game)
{
    // games can be launched using either a command common for the platform,
    // or one specific for the game
    const QString launch_cmd = game->m_launch_cmd.isEmpty()
        ? createLaunchCommand(collection, game)
        : game->m_launch_cmd;

    qInfo().noquote() << tr("Executing command: `%1`").arg(launch_cmd);

    beforeRun();
    runProcess(launch_cmd);
    afterRun();

    emit processFinished();
}

QString ProcessLauncher::createLaunchCommand(const Types::Collection* collection, const Types::Game* game)
{
    enum class ParamType : unsigned char {
        PATH,
        BASENAME,
    };

    QMap<ParamType, QString> params = {
        { ParamType::PATH, game->m_fileinfo.filePath() },
        { ParamType::BASENAME, game->m_fileinfo.completeBaseName() },
    };

    // Prepare the parameters for QProcess
    const QRegularExpression WHITESPACE_REGEX("\\s");
    for (auto& param : params) {
        // QProcess: Literal quotes are represented by triple quotes
        param.replace('"', R"(""")");
        // QProcess: Arguments containing spaces must be quoted
        if (param.contains(WHITESPACE_REGEX))
            param.prepend('"').append('"');
    }

    // replace known keywords
    QString launch_cmd = collection->launchCmd();
    // first, replace manually quoted elements in the command string (see Qt docs)
    // TODO: ES2 compatibility should be handled in its file
    launch_cmd
        .replace("\"%ROM%\"", params.value(ParamType::PATH))
        .replace("\"%ROM_RAW%\"", params.value(ParamType::PATH))
        .replace("\"%BASENAME%\"", params.value(ParamType::BASENAME));
    // then replace the unquoted forms
    launch_cmd
        .replace("%ROM%", params.value(ParamType::PATH))
        .replace("%ROM_RAW%", params.value(ParamType::PATH))
        .replace("%BASENAME%", params.value(ParamType::BASENAME));

    return launch_cmd;
}

void ProcessLauncher::runProcess(const QString& command)
{
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
    qInfo().noquote() << tr("Process %1 started").arg(process->processId());
    qInfo().noquote() << SEPARATOR;
}

void ProcessLauncher::onProcessFailed(QProcess::ProcessError error)
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
            // We don't communicate with the launched process at the moment
            Q_UNREACHABLE();
            break;
        default:
            qWarning().noquote() << tr("Running the command `%1` failed due to an unknown error")
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
            qInfo().noquote() << tr("The external program has finished cleanly, with exit code %2")
                                 .arg(exitcode);
            break;
        case QProcess::CrashExit:
            qInfo().noquote() << tr("The external program has crashed on exit, with exit code %2")
                                 .arg(exitcode);
            break;
        default:
            // If you reach this branch, there was an API change in Qt
            Q_UNREACHABLE();
            break;
    }
}

void ProcessLauncher::beforeRun()
{
    // call the relevant scripts
    using ScriptEvent = ScriptRunner::EventType;
    ScriptRunner::findAndRunScripts(ScriptEvent::PROCESS_STARTED);
}

void ProcessLauncher::afterRun()
{
    // call the relevant scripts
    using ScriptEvent = ScriptRunner::EventType;
    ScriptRunner::findAndRunScripts(ScriptEvent::PROCESS_FINISHED);
}
