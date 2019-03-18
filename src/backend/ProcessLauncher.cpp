// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "platform/TerminalKbd.h"
#include "utils/CommandTokenizer.h"

#include <QDebug>
#include <QDir>
#include <QRegularExpression>


namespace {
static constexpr auto SEPARATOR = "----------------------------------------";

void format_replace_env(QString& launch_cmd)
{
    const auto env = QProcessEnvironment::systemEnvironment();
    const QRegularExpression rx_env(QStringLiteral("{env.([^}]+)}"));

    auto match = rx_env.match(launch_cmd);
    while (match.hasMatch()) {
        const int from = match.capturedStart();
        const int len = match.capturedLength();
        launch_cmd.replace(from, len, env.value(match.captured(1)));

        const int match_offset = match.capturedStart() + match.capturedLength();
        match = rx_env.match(launch_cmd, match_offset);
    }
}

void format_launch_command(QStringList& launch_args, const QFileInfo& finfo)
{
    for (QString& part : launch_args) {
        part
            .replace(QLatin1String("{file.path}"), QDir::toNativeSeparators(finfo.absoluteFilePath()))
            .replace(QLatin1String("{file.name}"), finfo.fileName())
            .replace(QLatin1String("{file.basename}"), finfo.completeBaseName())
            .replace(QLatin1String("{file.dir}"), QDir::toNativeSeparators(finfo.absolutePath()));

        format_replace_env(part);
    }
}

bool contains_slash(const QString& str)
{
    return str.contains(QChar('/')) || str.contains(QChar('\\'));
}

QString abs_launchcmd(const QString& cmd, const QString& base_dir)
{
    if (!contains_slash(cmd))
        return cmd;
    if (base_dir.isEmpty())
        return cmd;
    if (QFileInfo(cmd).isAbsolute())
        return cmd;

    return QDir::toNativeSeparators(base_dir % QChar('/') % cmd);
}

QString abs_workdir(const QString& workdir, const QString& base_dir, const QString& fallback_workdir)
{
    if (workdir.isEmpty())
        return fallback_workdir;
    if (base_dir.isEmpty())
        return workdir;
    if (QFileInfo(workdir).isAbsolute())
        return workdir;

    return QDir::toNativeSeparators(base_dir % QChar('/') % workdir);
}
} // namespace


ProcessLauncher::ProcessLauncher(QObject* parent)
    : QObject(parent)
    , m_process(nullptr)
{}

void ProcessLauncher::onLaunchRequested(const model::GameFile* q_gamefile)
{
    Q_ASSERT(q_gamefile);

    const auto q_game = static_cast<const model::Game* const>(q_gamefile->parent());
    const modeldata::Game& game = q_game->data();
    const modeldata::GameFile& gamefile = q_gamefile->data();


    // TODO: in the future, check the gamefile's own launch command first

    QStringList launch_args = ::utils::tokenize_command(game.launch_cmd);

    if (!launch_args.isEmpty())
        format_launch_command(launch_args, gamefile.fileinfo);

    if (launch_args.isEmpty() || launch_args.constFirst().isEmpty()) {
        qInfo().noquote()
            << tr_log("Cannot launch the game `%1` because there is no launch command defined for it!")
               .arg(game.title);
        emit processLaunchError();
        return;
    }


    const QString command = abs_launchcmd(launch_args.takeFirst(), game.relative_basedir);
    const QString workdir = abs_workdir(game.launch_workdir, game.relative_basedir, gamefile.fileinfo.absolutePath());

    beforeRun();
    runProcess(command, launch_args, workdir);
}

void ProcessLauncher::runProcess(const QString& command, const QStringList& args, const QString& workdir)
{
    qInfo().noquote() << tr_log("Executing command: [`%1`,`%2`]").arg(command, args.join(QLatin1String("`,`")));
    qInfo().noquote() << tr_log("Working directory: `%3`").arg(workdir);

    Q_ASSERT(!m_process);
    m_process = new QProcess(this);

    // set up signals and slots
    connect(m_process, &QProcess::started, this, &ProcessLauncher::onProcessStarted);
    connect(m_process, &QProcess::errorOccurred, this, &ProcessLauncher::onProcessFailed);
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &ProcessLauncher::onProcessFinished);

    // run the command
    m_process->setProcessChannelMode(QProcess::ForwardedChannels);
    m_process->setInputChannelMode(QProcess::ForwardedInputChannel);
    m_process->setWorkingDirectory(workdir);
    m_process->start(command, args, QProcess::ReadOnly);

    // wait
    const bool started_successfully = m_process->waitForStarted(-1);
    if (started_successfully) {
        emit processLaunchOk();
    }
    else {
        emit processLaunchError();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void ProcessLauncher::onTeardownComplete()
{
    Q_ASSERT(m_process);

    m_process->waitForFinished(-1);
    m_process->deleteLater();
    m_process = nullptr;
    emit processFinished();
}

void ProcessLauncher::onProcessStarted()
{
    Q_ASSERT(m_process);
    qInfo().noquote() << tr_log("Process %1 started").arg(m_process->processId());
    qInfo().noquote() << SEPARATOR;
}

void ProcessLauncher::onProcessFailed(QProcess::ProcessError error)
{
    Q_ASSERT(m_process);
    switch (error) {
        case QProcess::FailedToStart:
            qWarning().noquote() << tr_log("Could not run the command `%1`; either the"
                                           " invoked program is missing, or you don't have"
                                           " the permission to run it.")
                                    .arg(m_process->program());
            break;
        case QProcess::Crashed:
            qWarning().noquote() << tr_log("The external program `%1` has crashed")
                                    .arg(m_process->program());
            break;
        case QProcess::Timedout:
            qWarning().noquote() << tr_log("The command `%1` has not started in a"
                                           " reasonable amount of time")
                                    .arg(m_process->program());
            break;
        case QProcess::ReadError:
        case QProcess::WriteError:
            // We don't communicate with the launched process at the moment
            Q_UNREACHABLE();
            break;
        default:
            qWarning().noquote() << tr_log("Running the command `%1` failed due to an unknown error")
                                    .arg(m_process->program());
            break;
    }
    afterRun();
}

void ProcessLauncher::onProcessFinished(int exitcode, QProcess::ExitStatus exitstatus)
{
    Q_ASSERT(m_process);
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
    afterRun();
}

void ProcessLauncher::beforeRun()
{
    TerminalKbd::enable();
    ScriptRunner::run(ScriptEvent::PROCESS_STARTED);
}

void ProcessLauncher::afterRun()
{
    ScriptRunner::run(ScriptEvent::PROCESS_FINISHED);
    TerminalKbd::disable();
}
