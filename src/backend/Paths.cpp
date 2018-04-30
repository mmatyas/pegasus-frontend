// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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


#include "Paths.h"

#include "AppArgs.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>

#include <functional>


namespace {

QString get_writable_dir(const QStandardPaths::StandardLocation type)
{
    Q_ASSERT(type == QStandardPaths::AppConfigLocation || type == QStandardPaths::CacheLocation);

    const QString dir_path = [type](){
        if (type == QStandardPaths::AppConfigLocation && AppArgs::portable_mode)
            return QCoreApplication::applicationDirPath() + QStringLiteral("/config");

        const QRegularExpression replace_regex(QStringLiteral("(/pegasus-frontend){2}$"));
        return QStandardPaths::writableLocation(type)
            .replace(replace_regex, QStringLiteral("/pegasus-frontend"));
    }();

    // the path is never empty for AppConfigLocation/CacheLocation, as per Qt docs
    Q_ASSERT(!dir_path.isEmpty());
    QDir(dir_path).mkpath(QLatin1String(".")); // does nothing if already exists

    return dir_path;
}

} // namespace


namespace paths {

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
        return env.value(QStringLiteral("PEGASUS_HOME"), QDir::homePath());
#endif
    }();
    return home_path;
}

QStringList configDirs()
{
    static const QStringList config_dir_paths = [](){
        QStringList paths(QLatin1String(":"));
        paths << QCoreApplication::applicationDirPath();

        if (!AppArgs::portable_mode) {
#ifdef INSTALL_DATADIR
            if (QFileInfo::exists(INSTALL_DATADIR))
                paths << QString(INSTALL_DATADIR);
#endif
            paths << QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
            paths << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
            paths.removeDuplicates();

            // do not add the organization name to the search path
            const QRegularExpression regex(QStringLiteral("(/pegasus-frontend){2}$"));
            paths.replaceInStrings(regex, QStringLiteral("/pegasus-frontend"));
        }

        return paths;
    }();

    return config_dir_paths;
}

QString writableConfigDir()
{
    static const QString config_dir = get_writable_dir(QStandardPaths::AppConfigLocation);
    return config_dir;
}

QString writableCacheDir()
{
    static const QString cache_dir = get_writable_dir(QStandardPaths::CacheLocation);
    return cache_dir;
}

QString configIniPath()
{
    static const QString ini_path = writableConfigDir() + QStringLiteral("/pegasus-frontend.ini");
    return ini_path;
}

} // namespace paths
