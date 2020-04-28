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
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "terminal_kbd/TerminalKbd.h"
#include "utils/CommandTokenizer.h"

#include <QDebug>
#include <QDir>
#include <QRegularExpression>


namespace {
static constexpr auto SEPARATOR = "----------------------------------------";

void replace_env_vars(QString& param)
{
    const auto env = QProcessEnvironment::systemEnvironment();
    const QRegularExpression rx_env(QStringLiteral("{env.([^}]+)}"));

    auto match = rx_env.match(param);
    while (match.hasMatch()) {
        const int from = match.capturedStart();
        const int len = match.capturedLength();
        param.replace(from, len, env.value(match.captured(1)));

        const int match_offset = match.capturedStart() + match.capturedLength();
        match = rx_env.match(param, match_offset);
    }
}

void replace_variables(QString& param, const QFileInfo& finfo)
{
    param
        .replace(QLatin1String("{file.path}"), QDir::toNativeSeparators(finfo.absoluteFilePath()))
        .replace(QLatin1String("{file.name}"), finfo.fileName())
        .replace(QLatin1String("{file.basename}"), finfo.completeBaseName())
        .replace(QLatin1String("{file.dir}"), QDir::toNativeSeparators(finfo.absolutePath()));

    replace_env_vars(param);
}

bool contains_slash(const QString& str)
{
    return str.contains(QChar('/')) || str.contains(QChar('\\'));
}

QString serialize_command(const QString& cmd, const QStringList& args)
{
    return (QStringList(QDir::toNativeSeparators(cmd)) + args).join(QLatin1String("`,`"));
}

QString processerror_to_string(QProcess::ProcessError error)
{
    switch (error) {
        case QProcess::FailedToStart:
            return tr_log("Could not launch `%1`. Either the program is missing, "
                          "or you don't have the permission to run it.");
        case QProcess::Crashed:
            return tr_log("The external program `%1` has crashed");
        case QProcess::Timedout:
            return tr_log("The command `%1` did not start in a reasonable amount of time");
        case QProcess::ReadError:
        case QProcess::WriteError:
            // We don't communicate with the launched process at the moment
            Q_UNREACHABLE();
            break;
        default:
            return tr_log("Running the command `%1` failed due to an unknown error");
    }
}
} // namespace


namespace helpers {
QString abs_launchcmd(const QString& cmd, const QString& base_dir)
{
    Q_ASSERT(!cmd.isEmpty());

    if (!contains_slash(cmd))
        return cmd;

    return QFileInfo(base_dir, cmd).absoluteFilePath();
}

QString abs_workdir(const QString& workdir, const QString& base_dir, const QString& fallback_workdir)
{
    if (workdir.isEmpty())
        return fallback_workdir;

    return QFileInfo(base_dir, workdir).absoluteFilePath();
}
} // namespace helpers


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

    QStringList args = ::utils::tokenize_command(game.launch_cmd);
    for (QString& arg : args)
        replace_variables(arg, gamefile.fileinfo);

    QString command = args.isEmpty() ? QString() : args.takeFirst();
    if (command.isEmpty()) {
        const QString message = tr_log("Cannot launch the game `%1` because there is no launch command defined for it.")
            .arg(game.title);
        qWarning().noquote() << message;
        emit processLaunchError(message);
        return;
    }
    command = helpers::abs_launchcmd(command, game.relative_basedir);


    const QString default_workdir = contains_slash(command)
        ? QFileInfo(command).absolutePath()
        : gamefile.fileinfo.absolutePath();

    QString workdir = game.launch_workdir;
    replace_variables(workdir, gamefile.fileinfo);
    workdir = helpers::abs_workdir(workdir, game.relative_basedir, default_workdir);


    beforeRun(gamefile.fileinfo.absoluteFilePath());
    runProcess(command, args, workdir);
}

void ProcessLauncher::runProcess(const QString& command, const QStringList& args, const QString& workdir)
{
    qInfo().noquote() << tr_log("Executing command: [`%1`]").arg(serialize_command(command, args));
    qInfo().noquote() << tr_log("Working directory: `%3`").arg(QDir::toNativeSeparators(workdir));

    Q_ASSERT(!m_process);
    m_process = new QProcess(this);

    // set up signals and slots
    connect(m_process, &QProcess::started, this, &ProcessLauncher::onProcessStarted);
    connect(m_process, &QProcess::errorOccurred, this, &ProcessLauncher::onProcessError);
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &ProcessLauncher::onProcessFinished);

    // run the command
    m_process->setProcessChannelMode(QProcess::ForwardedChannels);
    m_process->setInputChannelMode(QProcess::ForwardedInputChannel);
    m_process->setWorkingDirectory(workdir);
    m_process->start(command, args, QProcess::ReadOnly);
    m_process->waitForStarted(-1);
}

void ProcessLauncher::onTeardownComplete()
{
    Q_ASSERT(m_process);

    m_process->waitForFinished(-1);
    emit processFinished();
}

void ProcessLauncher::onProcessStarted()
{
    Q_ASSERT(m_process);
    qInfo().noquote() << tr_log("Process %1 started").arg(m_process->processId());
    qInfo().noquote() << SEPARATOR;
    emit processLaunchOk();
}

void ProcessLauncher::onProcessError(QProcess::ProcessError error)
{
    Q_ASSERT(m_process);

    const QString message = processerror_to_string(error).arg(m_process->program());

    switch (m_process->state()) {
        case QProcess::Starting:
        case QProcess::NotRunning:
            emit processLaunchError(message);
            qWarning().noquote() << message;
            afterRun(); // finished() won't run
            break;

        case QProcess::Running:
            emit processRuntimeError(message);
            break;
    }
}

void ProcessLauncher::onProcessFinished(int exitcode, QProcess::ExitStatus exitstatus)
{
    Q_ASSERT(m_process);
    qInfo().noquote() << SEPARATOR;

    switch (exitstatus) {
        case QProcess::NormalExit:
            if (exitcode == 0)
                qInfo().noquote() << tr_log("The external program has finished cleanly");
            else
                qWarning().noquote() << tr_log("The external program has finished with error code %1").arg(exitcode);
            break;
        case QProcess::CrashExit:
            qWarning().noquote() << tr_log("The external program has crashed");
            break;
    }

    afterRun();
}

void ProcessLauncher::beforeRun(const QString& game_path)
{
    TerminalKbd::enable();
    ScriptRunner::run(ScriptEvent::PROCESS_STARTED, { game_path });
}

void ProcessLauncher::afterRun()
{
    Q_ASSERT(m_process);
    m_process->deleteLater();
    m_process = nullptr;

    ScriptRunner::run(ScriptEvent::PROCESS_FINISHED);
    TerminalKbd::disable();
}
