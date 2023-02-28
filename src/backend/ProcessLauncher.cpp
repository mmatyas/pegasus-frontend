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

#include "Log.h"
#include "ScriptRunner.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "platform/TerminalKbd.h"
#include "utils/CommandTokenizer.h"
#include "utils/PathTools.h"

#ifdef Q_OS_ANDROID
#include "platform/AndroidHelpers.h"
#endif

#include <QDir>
#include <QUrl>
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
    const QString abs_path = finfo.absoluteFilePath();

#ifdef Q_OS_ANDROID
    const QString uri_str = android::to_content_uri(abs_path);
#else
    const QString uri_str = QUrl::fromLocalFile(abs_path).toString(QUrl::FullyEncoded);
#endif

    param
        .replace(QLatin1String("{file.path}"), ::pretty_path(finfo))
        .replace(QLatin1String("{file.uri}"), uri_str)
        .replace(QLatin1String("{file.name}"), finfo.fileName())
        .replace(QLatin1String("{file.basename}"), finfo.completeBaseName())
        .replace(QLatin1String("{file.dir}"), ::pretty_dir(finfo));

#ifdef Q_OS_ANDROID
    const QString document_uri_str = android::to_document_uri(abs_path);
    param.replace(QLatin1String("{file.documenturi}"), document_uri_str);
#endif

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
            return LOGMSG("Could not launch `%1`. Either the program is missing, "
                          "or you don't have the permission to run it.");
        case QProcess::Crashed:
            return LOGMSG("The external program `%1` has crashed");
        case QProcess::Timedout:
            return LOGMSG("The command `%1` did not start in a reasonable amount of time");
        case QProcess::ReadError:
        case QProcess::WriteError:
            // We don't communicate with the launched process at the moment
            Q_UNREACHABLE();
            break;
        default:
            return LOGMSG("Running the command `%1` failed due to an unknown error");
    }
}

#ifdef Q_OS_ANDROID
QString pretty_android_exception(const QString& error)
{
    if (error.startsWith(QLatin1String("android.content.ActivityNotFoundException"))) {
        return LOGMSG(
            "The Android activity you are trying to launch does not exist. "
            "Here is the full error message: %1"
        );
    }
    if (error.startsWith(QLatin1String("java.lang.SecurityException:"))) {
        return LOGMSG(
            "The Android system refused to run the launch command. "
            "This usually happens when you try to use native paths on Android 10 or later. "
            "Here is the full error message: %1"
        );
    }
    return LOGMSG("Failed to run the launch command: %1");
}
#endif // Q_OS_ANDROID
} // namespace


namespace helpers {
QString abs_launchcmd(const QString& cmd, const QString& base_dir)
{
    Q_ASSERT(!cmd.isEmpty());

    if (!contains_slash(cmd))
        return cmd;

    return ::clean_abs_path(QFileInfo(base_dir, cmd));
}

QString abs_workdir(const QString& workdir, const QString& base_dir, const QString& fallback_workdir)
{
    if (workdir.isEmpty())
        return fallback_workdir;

    return ::clean_abs_path(QFileInfo(base_dir, workdir));
}
} // namespace helpers


ProcessLauncher::ProcessLauncher(QObject* parent)
    : QObject(parent)
    , m_process(nullptr)
{}

void ProcessLauncher::onLaunchRequested(const model::GameFile* q_gamefile)
{
    Q_ASSERT(q_gamefile);

    const model::GameFile& gamefile = *q_gamefile;
    const model::Game& game = *gamefile.parentGame();

    const QString raw_launch_cmd =
#if defined(Q_OS_LINUX) && defined(PEGASUS_INSIDE_FLATPAK)
        QLatin1String("flatpak-spawn --host ") + game.launchCmd();
#else
        game.launchCmd();
#endif


    // TODO: in the future, check the gamefile's own launch command first

    QStringList args = ::utils::tokenize_command(raw_launch_cmd);
    for (QString& arg : args)
        replace_variables(arg, gamefile.fileinfo());

    QString command = args.isEmpty() ? QString() : args.takeFirst();
    if (command.isEmpty()) {
        const QString message = LOGMSG("Cannot launch the game `%1` because there is no launch command defined for it.")
            .arg(game.title());
        Log::warning(message);
        emit processLaunchError(message);
        return;
    }
    command = helpers::abs_launchcmd(command, game.launchCmdBasedir());

#ifdef Q_OS_ANDROID
    const bool android_command_valid = command.toLower() == QLatin1String("am");
    const bool android_args_valid = !args.isEmpty() && args.first().toLower() == QLatin1String("start");
    if (!android_command_valid || !android_args_valid) {
        const QString message = LOGMSG("Only 'am start' commands are supported at the moment");
        Log::warning(message);
        emit processLaunchError(message);
        return;
    }
#endif

#ifdef Q_OS_WINDOWS
    const QFileInfo command_finfo(command);
    if (command_finfo.isShortcut()) {
        args = QStringList {
            QStringLiteral("/q"),
            QStringLiteral("/c"),
            command,
        } + args;
        command = QStringLiteral("cmd");
    }
#endif

    const QString default_workdir = contains_slash(command)
        ? QFileInfo(command).absolutePath()
        : gamefile.fileinfo().absolutePath();

    QString workdir = game.launchWorkdir();
    replace_variables(workdir, gamefile.fileinfo());
    workdir = helpers::abs_workdir(workdir, game.launchCmdBasedir(), default_workdir);


    beforeRun(gamefile.fileinfo().absoluteFilePath());
    runProcess(command, args, workdir);
}

void ProcessLauncher::runProcess(const QString& command, const QStringList& args, const QString& workdir)
{
    Log::info(LOGMSG("Executing command: [`%1`]").arg(serialize_command(command, args)));
    Log::info(LOGMSG("Working directory: `%3`").arg(::pretty_path(workdir)));

#ifndef Q_OS_ANDROID
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

#else // Q_OS_ANDROID
    const QString result = android::run_am_call(args);
    if (result.isEmpty()) {
        emit processLaunchOk();
        Log::info(LOGMSG("Activity finished"));
    }
    else {
        const QString message = pretty_android_exception(result).arg(result);
        emit processLaunchError(message);
        Log::warning(message);
        afterRun();
    }

#endif // Q_OS_ANDROID
}

void ProcessLauncher::onTeardownComplete()
{
#ifndef Q_OS_ANDROID
    Q_ASSERT(m_process);
    m_process->waitForFinished(-1);
#endif
    emit processFinished();
}

void ProcessLauncher::onProcessStarted()
{
    Q_ASSERT(m_process);
    Log::info(LOGMSG("Process %1 started").arg(m_process->processId()));
    Log::info(SEPARATOR);
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
            Log::warning(message);
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
    Log::info(SEPARATOR);

    switch (exitstatus) {
        case QProcess::NormalExit:
            if (exitcode == 0)
                Log::info(LOGMSG("The external program has finished cleanly"));
            else
                Log::warning(LOGMSG("The external program has finished with error code %1").arg(exitcode));
            break;
        case QProcess::CrashExit:
            Log::warning(LOGMSG("The external program has crashed"));
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
#ifndef Q_OS_ANDROID
    Q_ASSERT(m_process);
    m_process->deleteLater();
    m_process = nullptr;
#endif

    ScriptRunner::run(ScriptEvent::PROCESS_FINISHED);
    TerminalKbd::disable();
}
