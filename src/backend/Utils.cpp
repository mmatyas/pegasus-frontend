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


#include "Utils.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QVector>

#ifdef Q_OS_UNIX
#include <sys/stat.h>
#endif

#include <unordered_map>


bool validPath(const QString& path) {
#ifdef Q_OS_UNIX
    // fast posix check for unix systems
    static struct ::stat buffer;
    return (::stat(path.toUtf8().constData(), &buffer) == 0);
#else
    // default Qt fallback
    QFileInfo file(path);
    return file.exists() && (file.isFile() || file.isDir());
#endif
}

bool validFileQt(const QString& path)
{
    QFileInfo file(path);
    return file.exists() && file.isFile();
}

QString homePath()
{
    static const QString home_path = [](){
        const auto env = QProcessEnvironment::systemEnvironment();

#ifdef Q_OS_WIN32
        // allow overriding the home directory on Windows:
        // QDir::homePath() checks the env vars last on this platform,
        // but we want it to be the first
        return env.value("PEGASUS_HOME", env.value("HOME", QDir::homePath()));
#else
        // on other platforms, QDir::homePath() returns $HOME first
        return env.value("PEGASUS_HOME", QDir::homePath());
#endif
    }();
    return home_path;
}

QStringList configDirPaths()
{
    static const QStringList config_dir_paths = [](){
        QStringList paths(QLatin1String(":"));
        paths << QCoreApplication::applicationDirPath();
#ifdef INSTALL_DATADIR
        if (validPath(INSTALL_DATADIR))
            paths << QString(INSTALL_DATADIR);
#endif
        paths << QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
        paths << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
        paths.removeDuplicates();

        // do not add the organization name to the search path
        const QRegularExpression regex(QStringLiteral("/pegasus-frontend/pegasus-frontend$"));
        paths.replaceInStrings(regex, QStringLiteral("/pegasus-frontend"));

        return paths;
    }();

    return config_dir_paths;
}

const std::function<int(int,int)>& shifterFn(IndexShiftDirection direction)
{
    static const std::unordered_map<IndexShiftDirection, std::function<int(int,int)>, EnumClassHash> fn_table {
        { IndexShiftDirection::INCREMENT,        [](int idx, int count){ return mathMod(idx + 1, count); } },
        { IndexShiftDirection::DECREMENT,        [](int idx, int count){ return mathMod(idx - 1, count); } },
        { IndexShiftDirection::INCREMENT_NOWRAP, [](int idx, int count){ return std::min(idx + 1, count - 1); } },
        { IndexShiftDirection::DECREMENT_NOWRAP, [](int idx, int)      { return std::max(idx - 1, 0); } },
    };
    return fn_table.at(direction);
}
