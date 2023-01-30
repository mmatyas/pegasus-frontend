// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include "Log.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"

#include <QDirIterator>
#include <QStringBuilder>
#include <QTextStream>


namespace providers {
namespace steam {

Gamelist::Gamelist(QString log_tag)
    : m_log_tag(std::move(log_tag))
    , m_name_filters { QStringLiteral("appmanifest_*.acf") }
    , m_ignored_manifests {
          QLatin1String("appmanifest_228980.acf"), // Steamworks Common Redistributables
          QLatin1String("appmanifest_996510.acf"), // Proton 3.16 Beta
          QLatin1String("appmanifest_961940.acf"), // Proton 3.16
          QLatin1String("appmanifest_930400.acf"), // Proton 3.7 Beta
          QLatin1String("appmanifest_858280.acf"), // Proton 3.7
          QLatin1String("appmanifest_1054830.acf"), // Proton 4.2
          QLatin1String("appmanifest_1070560.acf"), // Steam Linux Runtime
          QLatin1String("appmanifest_1113280.acf"), // Proton 4.11
          QLatin1String("appmanifest_1245040.acf"), // Proton 5.0
          QLatin1String("appmanifest_1391110.acf"), // Steam Linux Runtime - Soldier
          QLatin1String("appmanifest_1628350.acf"), // Steam Linux Runtime - Sniper
          QLatin1String("appmanifest_1420170.acf"), // Proton 5.13
          QLatin1String("appmanifest_1580130.acf"), // Proton 6.3
          QLatin1String("appmanifest_1887720.acf"), // Proton 7.0
          QLatin1String("appmanifest_1493710.acf"), // Proton Experimental
          QLatin1String("appmanifest_2180100.acf"), // Proton Hotfix
          QLatin1String("appmanifest_2230260.acf"), // Proton Next
          QLatin1String("appmanifest_1826330.acf"), // Proton EasyAntiCheat Runtime
          QLatin1String("appmanifest_1161040.acf"), // Proton BattlEye Runtime
    }
    , m_rx_acf_appid(QStringLiteral(R""("appid"\s+"(\d+)")""), QRegularExpression::CaseInsensitiveOption)
    , m_rx_acf_title(QStringLiteral(R""("name"\s+"([^"]+)")""))
{}

std::pair<QString, QString> Gamelist::read_manifest_file(const QString& manifest_path) const
{
    QFile manifest(manifest_path);
    if (!manifest.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Log::error(m_log_tag, LOGMSG("Could not open `%1`").arg(manifest_path));
        return {};
    }

    QString appid;
    QString title;

    QTextStream stream(&manifest);
    stream.setCodec("UTF-8");

    while (!stream.atEnd() && (appid.isEmpty() || title.isEmpty())) {
        const QString line = stream.readLine();

        const auto appid_match = m_rx_acf_appid.match(line);
        if (appid_match.hasMatch()) {
            appid = appid_match.captured(1);
            continue;
        }

        const auto title_match = m_rx_acf_title.match(line);
        if (title_match.hasMatch()) {
            title = title_match.captured(1);
        }
    }

    return { appid, title };
}

HashMap<QString, model::Game*> Gamelist::find_in(
    const QString& steam_call,
    const QString& dir_path,
    model::Collection& collection,
    SearchContext& sctx) const
{
    HashMap<QString, model::Game*> result;

    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::FollowSymlinks;

    QDirIterator dir_it(dir_path, m_name_filters, dir_filters, dir_flags);
    while (dir_it.hasNext()) {
        const QString manifest_path = dir_it.next();
        const QFileInfo finfo = dir_it.fileInfo();

        const auto it = std::find(m_ignored_manifests.cbegin(), m_ignored_manifests.cend(), finfo.fileName());
        if (it != m_ignored_manifests.cend())
            continue;

        // TODO: C++17
        QString appid;
        QString title;
        std::tie(appid, title) = read_manifest_file(manifest_path);
        if (appid.isEmpty())
            continue;
        if (title.isEmpty())
            title = QLatin1String("App #") + appid;

        const QString steam_uri = QStringLiteral("steam:") + appid;
        model::Game* game_ptr = sctx.game_by_uri(steam_uri);
        if (!game_ptr) {
            game_ptr = sctx.create_game_for(collection);
            sctx.game_add_uri(*game_ptr, steam_uri);
        }
        sctx.game_add_to(*game_ptr, collection);

        (*game_ptr)
            .setTitle(title)
            .setSortBy(title)
            .setLaunchCmd(steam_call % QLatin1String(" steam://rungameid/") % appid);

        result.emplace(appid, game_ptr);
    }

    return result;
}

} // namespace steam
} // namespace providers
