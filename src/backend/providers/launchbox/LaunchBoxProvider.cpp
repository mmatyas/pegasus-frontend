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


#include "LaunchBoxProvider.h"

#include "LaunchBoxCommon.h"
#include "LaunchBoxEmulatorsXml.h"
#include "LaunchBoxGamelistXml.h"
#include "LaunchBoxPlatformsXml.h"
#include "LocaleUtils.h"
#include "Paths.h"
#include "model/gaming/Game.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>
#include <QStringBuilder>


namespace {

HashMap<QString, model::Game*>
build_escaped_title_map(const QVector<model::Game*>& coll_childs)
{
    const QRegularExpression rx_invalid(QStringLiteral(R"([<>:"\/\\|?*'])"));
    const QString underscore(QLatin1Char('_'));

    HashMap<QString, model::Game*> out;
    for (model::Game* const game : coll_childs) {
        QString title = game->title();
        title.replace(rx_invalid, underscore);
        out.emplace(std::move(title), game);
    }
    return out;
}

void find_assets_in(const QString& asset_dir,
                    const AssetType asset_type,
                    const bool has_num_suffix,
                    const HashMap<QString, model::Game*>& title_to_game_map)
{
    constexpr auto files_only = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto recursive = QDirIterator::Subdirectories;

    QDirIterator file_it(asset_dir, files_only, recursive);
    while (file_it.hasNext()) {
        file_it.next();

        const QString basename = file_it.fileInfo().completeBaseName();
        const QString game_title = has_num_suffix
            ? basename.left(basename.length() - 3) // gamename "-xx" .ext
            : basename;

        const auto it = title_to_game_map.find(game_title);
        if (it == title_to_game_map.cend())
            continue;

        model::Game& game = *it->second;
        game.assets().add_file(asset_type, file_it.filePath());
    }
}

void find_assets(const QString& lb_dir, const QString& platform_name,
                 const std::vector<std::pair<QString, AssetType>>& assetdir_map,
                 providers::SearchContext& sctx)
{
    const auto coll_it = sctx.collections.find(platform_name);
    if (coll_it == sctx.collections.cend() || coll_it->second->gamesConst().isEmpty())
        return;

    const model::Collection& collection = *coll_it->second;
    const HashMap<QString, model::Game*> esctitle_to_game_map = build_escaped_title_map(collection.gamesConst());

    const QString images_root = lb_dir % QLatin1String("Images/") % platform_name % QLatin1Char('/');
    for (const auto& assetdir_pair : assetdir_map) {
        const QString assetdir_path = images_root + assetdir_pair.first;
        const AssetType assetdir_type = assetdir_pair.second;
        find_assets_in(assetdir_path, assetdir_type, true, esctitle_to_game_map);
    }

    const QString music_root = lb_dir % QLatin1String("Music/") % platform_name % QLatin1Char('/');
    find_assets_in(music_root, AssetType::MUSIC, false, esctitle_to_game_map);

    const QString video_root = lb_dir % QLatin1String("Videos/") % platform_name % QLatin1Char('/');
    find_assets_in(video_root, AssetType::VIDEO, false, esctitle_to_game_map);
}

QString find_installation()
{
    const QString possible_path = paths::homePath() + QStringLiteral("/LaunchBox/");
    if (QFileInfo::exists(possible_path)) {
        qInfo().noquote() << providers::launchbox::MSG_PREFIX
            << tr_log("found directory: `%1`").arg(QDir::toNativeSeparators(possible_path));
        return possible_path;
    }
    return {};
}
} // namespace


namespace providers {
namespace launchbox {

LaunchboxProvider::LaunchboxProvider(QObject* parent)
    : Provider(QLatin1String("launchbox"), QStringLiteral("LaunchBox"), PROVIDES_GAMES, parent)
{}

void LaunchboxProvider::findLists(providers::SearchContext& sctx)
{
    const QString lb_dir = [this]{
        const auto option_it = options().find(QLatin1String("installdir"));
        return (option_it != options().cend())
            ? QDir::cleanPath(option_it->second.front()) + QLatin1Char('/')
            : find_installation();
    }();
    if (lb_dir.isEmpty()) {
        qInfo().noquote() << MSG_PREFIX << tr_log("no installation found");
        return;
    }

    const std::vector<QString> platform_names = platforms_xml::read(lb_dir);
    if (platform_names.empty()) {
        qWarning().noquote() << MSG_PREFIX << tr_log("no platforms found");
        return;
    }

    const HashMap<EmulatorId, Emulator> emulators = emulators_xml::read(lb_dir);
    if (emulators.empty()) {
        qWarning().noquote() << MSG_PREFIX << tr_log("no emulator settings found");
        return;
    }

    const Literals literals;
    for (const QString& platform_name : platform_names) {
        gamelist_xml::read(literals, lb_dir, platform_name, emulators, sctx);
        find_assets(lb_dir, platform_name, literals.assetdir_map, sctx);
    }
}

} // namespace launchbox
} // namespace providers
