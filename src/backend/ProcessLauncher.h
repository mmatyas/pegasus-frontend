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


#pragma once

#include "utils/FwdDeclModel.h"

#include <QObject>
#include <QProcess>


namespace helpers {
QString abs_launchcmd(const QString& cmd, const QString& base_dir);
QString abs_workdir(const QString& workdir, const QString& base_dir, const QString& fallback_workdir);
} // namespace helpers


/// Launches and manages external processes
///
/// Launches external processes and detects their success or failure.
class ProcessLauncher : public QObject {
    Q_OBJECT

public:
    explicit ProcessLauncher(QObject* parent = nullptr);

signals:
    void processLaunchOk();
    void processLaunchError(const QString);
    void processFinished();

public slots:
    void onLaunchRequested(const model::GameFile*);
    void onTeardownComplete();

private slots:
    void onProcessStarted();
    void onProcessFailed(QProcess::ProcessError);
    void onProcessFinished(int, QProcess::ExitStatus);

private:
    QProcess* m_process;

    void runProcess(const QString&, const QStringList&, const QString&);

    void beforeRun();
    void afterRun();
};
