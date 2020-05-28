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


#include "SteamGamelist.h"

#include "LocaleUtils.h"
#include "Paths.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QStringBuilder>
#include <array>


namespace {
static constexpr auto MSG_PREFIX = "Steam:";

QString find_steam_datadir()
{
    QStringList possible_dirs;

#ifdef Q_OS_UNIX
    // Linux: ~/.local/share/Steam
    // macOS: ~/Library/Application Support/Steam
    possible_dirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (QString& dir : possible_dirs)
        dir.append(QLatin1String("/Steam/"));

#ifdef Q_OS_LINUX
    // in addition on Linux, ~/.steam/steam
    possible_dirs << paths::homePath() % QLatin1String("/.steam/steam/");
#endif // linux
#endif // unix

#ifdef Q_OS_WIN
    QSettings reg_base(QLatin1String("HKEY_CURRENT_USER\\Software\\Valve\\Steam"),
                       QSettings::NativeFormat);
    const QString reg_value = reg_base.value(QLatin1String("SteamPath")).toString();
    if (!reg_value.isEmpty())
        possible_dirs << reg_value % QChar('/');
#endif


    for (const auto& dir : qAsConst(possible_dirs)) {
        if (QFileInfo::exists(dir)) {
            qInfo().noquote() << MSG_PREFIX << tr_log("found data directory: `%1`").arg(dir);
            return dir;
        }
    }

    qInfo().noquote() << MSG_PREFIX << tr_log("no installation found");
    return {};
}

std::vector<QString> find_steam_installdirs(const QString& steam_datadir)
{
    std::vector<QString> installdirs;
    installdirs.emplace_back(steam_datadir % QLatin1String("steamapps"));


    const QString config_path = steam_datadir % QLatin1String("config/config.vdf");
    QFile configfile(config_path);
    if (!configfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("while Steam seems to be installed, "
                      "the config file `%1` could not be opened").arg(config_path);
        return installdirs;
    }

    const QRegularExpression installdir_regex(QStringLiteral(R""("BaseInstallFolder_\d+"\s+"([^"]+)")""));

    QTextStream stream(&configfile);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const auto match = installdir_regex.match(line);
        if (match.hasMatch()) {
            const QString path = match.captured(1) % QLatin1String("/steamapps/");
            if (QFileInfo::exists(path))
                installdirs.emplace_back(path);
        }
    }

    installdirs.erase(std::unique(installdirs.begin(), installdirs.end()), installdirs.end());
    return installdirs;
}

bool should_ignore(const QString& filename)
{
    const std::array<QLatin1String, 9> ignored {
        QLatin1String("appmanifest_228980.acf"), // Steamworks Common Redistributables
        QLatin1String("appmanifest_996510.acf"), // Proton 3.16 Beta
        QLatin1String("appmanifest_961940.acf"), // Proton 3.16
        QLatin1String("appmanifest_930400.acf"), // Proton 3.7 Beta
        QLatin1String("appmanifest_858280.acf"), // Proton 3.7
        QLatin1String("appmanifest_1054830.acf"), // Proton 4.2
        QLatin1String("appmanifest_1070560.acf"), // Steam Linux Runtime
        QLatin1String("appmanifest_1113280.acf"), // Proton 4.11
        QLatin1String("appmanifest_1245040.acf"), // Proton 5.0
    };
    return std::find(ignored.cbegin(), ignored.cend(), filename) != ignored.cend();
}

void register_appmanifests(providers::SearchContext& sctx,
                           providers::PendingCollection& collection,
                           const std::vector<QString>& installdirs)
{
    const auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    const auto dir_flags = QDirIterator::FollowSymlinks;
    const QStringList name_filters { QStringLiteral("appmanifest_*.acf") };

    for (const QString& dir_path : installdirs) {
        QDirIterator dir_it(dir_path, name_filters, dir_filters, dir_flags);
        while (dir_it.hasNext()) {
            dir_it.next();

            QFileInfo fileinfo = dir_it.fileInfo();
            if (should_ignore(fileinfo.fileName()))
                continue;

            sctx.add_or_create_game_from_file(std::move(fileinfo), collection);
        }
    }
}

} // namespace


namespace providers {
namespace steam {

Gamelist::Gamelist(QObject* parent)
    : QObject(parent)
{}

void Gamelist::find(providers::SearchContext& sctx)
{
    const QString steamdir = find_steam_datadir();
    if (steamdir.isEmpty())
        return;

    const std::vector<QString> installdirs = find_steam_installdirs(steamdir);
    if (installdirs.empty()) {
        qWarning().noquote() << MSG_PREFIX << tr_log("no installation directories found");
        return;
    }

    PendingCollection& coll = sctx.get_or_create_collection(QStringLiteral("Steam"));

    const size_t game_count_before = sctx.games().size();
    register_appmanifests(sctx, coll, installdirs);
    if (game_count_before != sctx.games().size())
        emit gameCountChanged(static_cast<int>(sctx.games().size()));
}

} // namespace steam
} // namespace providers
