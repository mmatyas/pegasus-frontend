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


#include "SteamGamelist.h"

#include "types/Collection.h"
#include "types/Game.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QStringBuilder>


namespace {
static constexpr auto MSG_PREFIX = "Steam:";

QString find_steam_datadir()
{
    QStringList possible_dirs;

#ifdef Q_OS_UNIX
    using Paths = QStandardPaths;

    // Linux: ~/.local/share/Steam
    // macOS: ~/Library/Application Support/Steam
    possible_dirs = Paths::standardLocations(Paths::GenericDataLocation);
    for (QString& dir : possible_dirs)
        dir.append(QLatin1String("/Steam/"));

#ifdef Q_OS_LINUX
    // in addition on Linux, ~/.steam/steam
    possible_dirs << Paths::standardLocations(Paths::HomeLocation).first()
                     % QLatin1String("/.steam/steam/");
#endif
#endif // unix

#ifdef Q_OS_WIN
    QSettings reg_base(QLatin1String("HKEY_CURRENT_USER\\Software\\Valve\\Steam"),
                       QSettings::NativeFormat);
    const QString reg_value = reg_base.value(QLatin1String("SteamPath")).toString();
    if (!reg_value.isEmpty())
        possible_dirs << reg_value % QChar('/');
#endif


    for (const auto& dir : qAsConst(possible_dirs)) {
        if (QFileInfo(dir).isDir()) {
            qInfo().noquote() << MSG_PREFIX
                              << QObject::tr("found data directory: `%1`").arg(dir);
            return dir;
        }
    }

    qInfo().noquote() << MSG_PREFIX
                      << QObject::tr("no installation found");
    return QString();
}

QStringList find_steam_installdirs(const QString& steam_datadir)
{
    QStringList installdirs;
    installdirs << steam_datadir % QLatin1String("steamapps");


    const QString config_path = steam_datadir % QLatin1String("config/config.vdf");
    QFile configfile(config_path);
    if (!configfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning().noquote() << MSG_PREFIX
            << QObject::tr("while Steam seems to be installed, "
                           "the config file `%1` could not be opened").arg(config_path);
        return installdirs;
    }


    const QRegularExpression installdir_regex(R""("BaseInstallFolder_\d+"\s+"([^"]+)")"");

    QTextStream stream(&configfile);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const auto match = installdir_regex.match(line);
        if (match.hasMatch()) {
            const QString path = match.captured(1) % QLatin1String("/steamapps");
            if (!installdirs.contains(path))
                installdirs << path;
        }
    }

    return installdirs;
}

void register_appmanifests(QHash<QString, Types::Game*>& games, Types::Collection* collection)
{
    Q_ASSERT(collection);

    static constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    static constexpr auto dir_flags = QDirIterator::FollowSymlinks;
    const QStringList name_filters = { QStringLiteral("appmanifest_*.acf") };

    for (const QString& dir_path : collection->sourceDirs()) {
        QDirIterator dir_it(dir_path, name_filters, dir_filters, dir_flags);
        while (dir_it.hasNext()) {
            dir_it.next();
            QFileInfo fileinfo = dir_it.fileInfo();

            Types::Game*& game_ptr = games[fileinfo.canonicalFilePath()];
            if (!game_ptr)
                game_ptr = new Types::Game(fileinfo, collection);

            collection->gameListMut().addGame(game_ptr);
        }
    }
}

} // namespace


namespace providers {

void SteamGamelist::find(QHash<QString, Types::Game*>& games,
                         QHash<QString, Types::Collection*>& collections,
                         QVector<QString>&)
{
    const QString steamdir = find_steam_datadir();
    if (steamdir.isEmpty())
        return;

    QStringList installdirs = find_steam_installdirs(steamdir);
    installdirs.erase(std::remove_if(
        installdirs.begin(), installdirs.end(),
            [](const QString& dir){ return !QFileInfo(dir).isDir(); }),
        installdirs.end());
    if (installdirs.isEmpty()) {
        qWarning().noquote() << MSG_PREFIX << QObject::tr("no installation directories found");
        return;
    }


    const QString STEAM_TAG(QStringLiteral("steam"));
    Types::Collection*& collection = collections[STEAM_TAG];
    if (!collection)
        collection = new Types::Collection(STEAM_TAG); // TODO: check for fail

    collection->setName(QStringLiteral("Steam"));
    collection->sourceDirsMut().append(installdirs);


    register_appmanifests(games, collection);
}

} // namespace providers
