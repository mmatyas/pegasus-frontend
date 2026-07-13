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


#include "GameDataCache.h"

#include "Log.h"
#include "Paths.h"
#include "Provider.h"
#include "SearchContext.h"
#include "model/ObjectListModel.h"
#include "model/gaming/Assets.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "types/AssetType.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSaveFile>
#include <QStringList>

namespace {
constexpr int CACHE_SCHEMA_VERSION = 1;

QJsonArray string_list_to_json(const QStringList& values)
{
    QJsonArray out;
    for (const QString& value : values)
        out.append(value);
    return out;
}

void add_non_empty_array(QJsonObject& obj, const QString& key, const QJsonArray& values)
{
    if (!values.isEmpty())
        obj[key] = values;
}

QStringList json_to_string_list(const QJsonValue& value)
{
    QStringList out;
    const QJsonArray array = value.toArray();
    out.reserve(array.size());
    for (const QJsonValue& item : array)
        out.append(item.toString());
    return out;
}

QJsonObject assets_to_json(const model::Assets& assets)
{
    QJsonObject obj;
    add_non_empty_array(obj, QStringLiteral("box_front"), string_list_to_json(assets.boxFrontList()));
    add_non_empty_array(obj, QStringLiteral("box_back"), string_list_to_json(assets.boxBackList()));
    add_non_empty_array(obj, QStringLiteral("box_spine"), string_list_to_json(assets.boxSpineList()));
    add_non_empty_array(obj, QStringLiteral("box_full"), string_list_to_json(assets.boxFullList()));
    add_non_empty_array(obj, QStringLiteral("cartridge"), string_list_to_json(assets.cartridgeList()));
    add_non_empty_array(obj, QStringLiteral("logo"), string_list_to_json(assets.logoList()));
    add_non_empty_array(obj, QStringLiteral("poster"), string_list_to_json(assets.posterList()));
    add_non_empty_array(obj, QStringLiteral("marquee"), string_list_to_json(assets.marqueeList()));
    add_non_empty_array(obj, QStringLiteral("bezel"), string_list_to_json(assets.bezelList()));
    add_non_empty_array(obj, QStringLiteral("panel"), string_list_to_json(assets.panelList()));
    add_non_empty_array(obj, QStringLiteral("cabinet_left"), string_list_to_json(assets.cabinetLeftList()));
    add_non_empty_array(obj, QStringLiteral("cabinet_right"), string_list_to_json(assets.cabinetRightList()));
    add_non_empty_array(obj, QStringLiteral("tile"), string_list_to_json(assets.tileList()));
    add_non_empty_array(obj, QStringLiteral("banner"), string_list_to_json(assets.bannerList()));
    add_non_empty_array(obj, QStringLiteral("steam"), string_list_to_json(assets.steamList()));
    add_non_empty_array(obj, QStringLiteral("background"), string_list_to_json(assets.backgroundList()));
    add_non_empty_array(obj, QStringLiteral("music"), string_list_to_json(assets.musicList()));
    add_non_empty_array(obj, QStringLiteral("screenshot"), string_list_to_json(assets.screenshotList()));
    add_non_empty_array(obj, QStringLiteral("titlescreen"), string_list_to_json(assets.titlescreenList()));
    add_non_empty_array(obj, QStringLiteral("video"), string_list_to_json(assets.videoList()));
    return obj;
}

void add_asset_uris(model::Assets& assets, const QJsonObject& obj, const QString& key, const AssetType type)
{
    const QStringList values = json_to_string_list(obj.value(key));
    for (const QString& value : values) {
        if (!value.isEmpty())
            assets.add_uri(type, value);
    }
}

void assets_from_json(model::Assets& assets, const QJsonValue& value)
{
    const QJsonObject obj = value.toObject();
    add_asset_uris(assets, obj, QStringLiteral("box_front"), AssetType::BOX_FRONT);
    add_asset_uris(assets, obj, QStringLiteral("box_back"), AssetType::BOX_BACK);
    add_asset_uris(assets, obj, QStringLiteral("box_spine"), AssetType::BOX_SPINE);
    add_asset_uris(assets, obj, QStringLiteral("box_full"), AssetType::BOX_FULL);
    add_asset_uris(assets, obj, QStringLiteral("cartridge"), AssetType::CARTRIDGE);
    add_asset_uris(assets, obj, QStringLiteral("logo"), AssetType::LOGO);
    add_asset_uris(assets, obj, QStringLiteral("poster"), AssetType::POSTER);
    add_asset_uris(assets, obj, QStringLiteral("marquee"), AssetType::ARCADE_MARQUEE);
    add_asset_uris(assets, obj, QStringLiteral("bezel"), AssetType::ARCADE_BEZEL);
    add_asset_uris(assets, obj, QStringLiteral("panel"), AssetType::ARCADE_PANEL);
    add_asset_uris(assets, obj, QStringLiteral("cabinet_left"), AssetType::ARCADE_CABINET_L);
    add_asset_uris(assets, obj, QStringLiteral("cabinet_right"), AssetType::ARCADE_CABINET_R);
    add_asset_uris(assets, obj, QStringLiteral("tile"), AssetType::UI_TILE);
    add_asset_uris(assets, obj, QStringLiteral("banner"), AssetType::UI_BANNER);
    add_asset_uris(assets, obj, QStringLiteral("steam"), AssetType::UI_STEAMGRID);
    add_asset_uris(assets, obj, QStringLiteral("background"), AssetType::BACKGROUND);
    add_asset_uris(assets, obj, QStringLiteral("music"), AssetType::MUSIC);
    add_asset_uris(assets, obj, QStringLiteral("screenshot"), AssetType::SCREENSHOT);
    add_asset_uris(assets, obj, QStringLiteral("titlescreen"), AssetType::TITLESCREEN);
    add_asset_uris(assets, obj, QStringLiteral("video"), AssetType::VIDEO);
}

void add_file_fingerprint(QJsonArray& out, const QString& path)
{
    const QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile())
        return;

    QJsonObject item;
    item[QStringLiteral("path")] = fi.absoluteFilePath();
    item[QStringLiteral("size")] = QString::number(fi.size());
    item[QStringLiteral("mtime")] = QString::number(fi.lastModified().toMSecsSinceEpoch());
    out.append(item);
}

void add_metadata_fingerprints(QJsonArray& out, const QString& dir_path)
{
    const QDir dir(dir_path);
    if (!dir.exists())
        return;

    const QStringList names = {
        QStringLiteral("metadata.pegasus.txt"),
        QStringLiteral("metadata.txt"),
    };
    for (const QString& name : names)
        add_file_fingerprint(out, dir.absoluteFilePath(name));

    const QStringList filters = {
        QStringLiteral("*.metadata.pegasus.txt"),
        QStringLiteral("*.metadata.txt"),
    };
    QDirIterator it(dir.absolutePath(), filters, QDir::Files | QDir::NoSymLinks);
    while (it.hasNext())
        add_file_fingerprint(out, it.next());
}

QJsonObject collection_to_json(const model::Collection& collection)
{
    QJsonObject obj;
    obj[QStringLiteral("name")] = collection.name();
    obj[QStringLiteral("sort_by")] = collection.sortBy();
    obj[QStringLiteral("short_name")] = collection.shortName();
    obj[QStringLiteral("summary")] = collection.summary();
    obj[QStringLiteral("description")] = collection.description();
    obj[QStringLiteral("common_launch_cmd")] = collection.commonLaunchCmd();
    obj[QStringLiteral("common_launch_workdir")] = collection.commonLaunchWorkdir();
    obj[QStringLiteral("common_relative_basedir")] = collection.commonLaunchCmdBasedir();
    obj[QStringLiteral("assets")] = assets_to_json(collection.assets());
    return obj;
}

QJsonObject game_to_json(const model::Game& game)
{
    QJsonObject obj;
    obj[QStringLiteral("title")] = game.title();
    obj[QStringLiteral("sort_by")] = game.sortBy();
    obj[QStringLiteral("summary")] = game.summary();
    obj[QStringLiteral("description")] = game.description();
    add_non_empty_array(obj, QStringLiteral("developers"), string_list_to_json(game.developerListConst()));
    add_non_empty_array(obj, QStringLiteral("publishers"), string_list_to_json(game.publisherListConst()));
    add_non_empty_array(obj, QStringLiteral("genres"), string_list_to_json(game.genreListConst()));
    add_non_empty_array(obj, QStringLiteral("tags"), string_list_to_json(game.tagListConst()));
    obj[QStringLiteral("player_count")] = game.playerCount();
    obj[QStringLiteral("rating")] = game.rating();
    obj[QStringLiteral("release_date")] = game.releaseDate().toString(Qt::ISODate);
    obj[QStringLiteral("missing")] = game.isMissing();
    obj[QStringLiteral("launch_cmd")] = game.launchCmd();
    obj[QStringLiteral("launch_workdir")] = game.launchWorkdir();
    obj[QStringLiteral("relative_basedir")] = game.launchCmdBasedir();
    obj[QStringLiteral("assets")] = assets_to_json(game.assets());

    QJsonArray collection_names;
    if (game.collectionsModel()) {
        for (const model::Collection* const collection : game.collectionsModel()->entries()) {
            if (collection)
                collection_names.append(collection->name());
        }
    }
    add_non_empty_array(obj, QStringLiteral("collections"), collection_names);

    QJsonArray files;
    if (game.filesModel()) {
        for (const model::GameFile* const file : game.filesModel()->entries()) {
            if (!file)
                continue;

            QJsonObject file_obj;
            file_obj[QStringLiteral("path")] = file->path();
            file_obj[QStringLiteral("name")] = file->name();
            file_obj[QStringLiteral("uri")] = file->uri();
            files.append(file_obj);
        }
    }
    add_non_empty_array(obj, QStringLiteral("files"), files);

    return obj;
}
} // namespace

QString GameDataCache::cacheFilePath()
{
    return QDir(paths::writableCacheDir()).absoluteFilePath(QStringLiteral("gameindex-v1.json"));
}

QString GameDataCache::buildFingerprint(
    const providers::SearchContext& sctx,
    const std::vector<providers::Provider*>& providers)
{
    QJsonObject root;
    root[QStringLiteral("schema")] = CACHE_SCHEMA_VERSION;

    QJsonArray provider_ids;
    for (const providers::Provider* provider : providers) {
        if (provider)
            provider_ids.append(QString(provider->codename()));
    }
    root[QStringLiteral("providers")] = provider_ids;
    root[QStringLiteral("root_game_dirs")] = string_list_to_json(sctx.root_game_dirs());

    // Keep the fingerprint based only on inputs that are known before the
    // providers run. PegasusProvider may populate pegasus_game_dirs() during
    // scanning, so including it here would make load-time and save-time
    // fingerprints differ and cause permanent cache misses.
    QJsonArray metadata_files;
    for (const QString& dir : paths::configDirs())
        add_metadata_fingerprints(metadata_files, dir);
    for (const QString& dir : sctx.root_game_dirs())
        add_metadata_fingerprints(metadata_files, dir);
    root[QStringLiteral("metadata_files")] = metadata_files;

    const QJsonDocument doc(root);
    const QByteArray hash = QCryptographicHash::hash(doc.toJson(QJsonDocument::Compact), QCryptographicHash::Sha256).toHex();
    return QString::fromLatin1(hash);
}

bool GameDataCache::load(
    providers::SearchContext& sctx,
    const std::vector<providers::Provider*>& providers)
{
    const QString expected_fingerprint = buildFingerprint(sctx, providers);
    QFile file(cacheFilePath());
    if (!file.open(QIODevice::ReadOnly))
        return false;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject root = doc.object();
    if (root.value(QStringLiteral("schema")).toInt() != CACHE_SCHEMA_VERSION)
        return false;
    if (root.value(QStringLiteral("fingerprint")).toString() != expected_fingerprint){
        Log::info(LOGMSG("Ignoring game index cache: fingerprint mismatch"));
        return false;
    }

    const QJsonArray collections = root.value(QStringLiteral("collections")).toArray();
    for (const QJsonValue& value : collections) {
        const QJsonObject obj = value.toObject();
        const QString name = obj.value(QStringLiteral("name")).toString();
        if (name.isEmpty())
            continue;

        model::Collection* collection = sctx.get_or_create_collection(name);
        collection->setSortBy(obj.value(QStringLiteral("sort_by")).toString());
        collection->setShortName(obj.value(QStringLiteral("short_name")).toString());
        collection->setSummary(obj.value(QStringLiteral("summary")).toString());
        collection->setDescription(obj.value(QStringLiteral("description")).toString());
        collection->setCommonLaunchCmd(obj.value(QStringLiteral("common_launch_cmd")).toString());
        collection->setCommonLaunchWorkdir(obj.value(QStringLiteral("common_launch_workdir")).toString());
        collection->setCommonLaunchCmdBasedir(obj.value(QStringLiteral("common_relative_basedir")).toString());
        assets_from_json(collection->assetsMut(), obj.value(QStringLiteral("assets")));
    }

    const QJsonArray games = root.value(QStringLiteral("games")).toArray();
    for (const QJsonValue& value : games) {
        const QJsonObject obj = value.toObject();
        const QString title = obj.value(QStringLiteral("title")).toString();
        if (title.isEmpty())
            continue;

        model::Game* game = sctx.create_game();
        game->setTitle(title);
        game->setSortBy(obj.value(QStringLiteral("sort_by")).toString());
        game->setSummary(obj.value(QStringLiteral("summary")).toString());
        game->setDescription(obj.value(QStringLiteral("description")).toString());
        game->developerList().append(json_to_string_list(obj.value(QStringLiteral("developers"))));
        game->publisherList().append(json_to_string_list(obj.value(QStringLiteral("publishers"))));
        game->genreList().append(json_to_string_list(obj.value(QStringLiteral("genres"))));
        game->tagList().append(json_to_string_list(obj.value(QStringLiteral("tags"))));
        game->setPlayerCount(obj.value(QStringLiteral("player_count")).toInt(1));
        game->setRating(static_cast<float>(obj.value(QStringLiteral("rating")).toDouble(0.0)));
        game->setReleaseDate(QDate::fromString(obj.value(QStringLiteral("release_date")).toString(), Qt::ISODate));
        game->setMissing(obj.value(QStringLiteral("missing")).toBool(false));
        game->setLaunchCmd(obj.value(QStringLiteral("launch_cmd")).toString());
        game->setLaunchWorkdir(obj.value(QStringLiteral("launch_workdir")).toString());
        game->setLaunchCmdBasedir(obj.value(QStringLiteral("relative_basedir")).toString());
        assets_from_json(game->assetsMut(), obj.value(QStringLiteral("assets")));

        const QStringList collection_names = json_to_string_list(obj.value(QStringLiteral("collections")));
        for (const QString& collection_name : collection_names) {
            if (collection_name.isEmpty())
                continue;
            model::Collection* collection = sctx.get_or_create_collection(collection_name);
            sctx.game_add_to(*game, *collection);
        }

        const QJsonArray files = obj.value(QStringLiteral("files")).toArray();
        for (const QJsonValue& file_value : files) {
            const QJsonObject file_obj = file_value.toObject();
            const QString uri = file_obj.value(QStringLiteral("uri")).toString();
            const QString path = file_obj.value(QStringLiteral("path")).toString();
            model::GameFile* game_file = nullptr;
            if (!path.isEmpty())
                game_file = sctx.game_add_filepath(*game, path);
            else if (!uri.isEmpty())
                game_file = sctx.game_add_uri(*game, uri);

            if (game_file)
                game_file->setName(file_obj.value(QStringLiteral("name")).toString());
        }
    }

    Log::info(LOGMSG("Loaded game index cache"));
    return true;
}

void GameDataCache::save(
    const providers::SearchContext& sctx,
    const std::vector<providers::Provider*>& providers,
    const std::vector<model::Collection*>& collections,
    const std::vector<model::Game*>& games)
{
    QJsonObject root;
    root[QStringLiteral("schema")] = CACHE_SCHEMA_VERSION;
    root[QStringLiteral("fingerprint")] = buildFingerprint(sctx, providers);

    QJsonArray collection_array;
    for (const model::Collection* collection : collections) {
        if (collection)
            collection_array.append(collection_to_json(*collection));
    }
    add_non_empty_array(root, QStringLiteral("collections"), collection_array);

    QJsonArray game_array;
    for (const model::Game* game : games) {
        if (game)
            game_array.append(game_to_json(*game));
    }
    add_non_empty_array(root, QStringLiteral("games"), game_array);

    QDir().mkpath(paths::writableCacheDir());
    QSaveFile file(cacheFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        Log::warning(LOGMSG("Could not open game index cache for writing"));
        return;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    if (!file.commit())
        Log::warning(LOGMSG("Could not write game index cache"));
    else
        Log::info(LOGMSG("Saved game index cache"));
}

void GameDataCache::clear()
{
    QFile::remove(cacheFilePath());
}
