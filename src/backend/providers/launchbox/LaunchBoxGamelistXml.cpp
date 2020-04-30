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


#include "LaunchBoxGamelistXml.h"

#include "LaunchBoxCommon.h"
#include "LocaleUtils.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStringBuilder>


namespace providers {
namespace launchbox {

void add_raw_asset_maybe(modeldata::GameAssets& assets, const AssetType type,
                         const QString& lb_root, const QString& path)
{
    QString can_path = QFileInfo(lb_root, path).canonicalFilePath();
    if (can_path.isEmpty()) {
        qWarning().noquote()
            << tr_log("LaunchBox: asset path `%1` doesn't exist, ignored").arg(path);
        return;
    }

    assets.addFileMaybe(type, std::move(can_path));
}

void store_addiapp(
    const QString& lb_dir,
    const HashMap<AdditionalAppField, QString, EnumHash>& fields,
    const size_t gameid,
    providers::SearchContext& sctx)
{
    const QFileInfo finfo(lb_dir, fields.at(AdditionalAppField::PATH));
    const auto name_it = fields.find(AdditionalAppField::NAME);

    modeldata::Game& game = sctx.games.at(gameid);

    auto file_it = std::find_if(game.files.begin(), game.files.end(),
        [&finfo](const modeldata::GameFile& gf){ return gf.fileinfo == finfo; });

    // if it refers to an existing path, do not duplicate, just try to give it a name
    if (file_it == game.files.end()) {
        game.files.emplace_back(std::move(finfo));
        file_it = std::prev(game.files.end());
    }
    if (name_it != fields.cend())
        file_it->name = name_it->second;

    sctx.path_to_gameid.emplace(finfo.canonicalFilePath(), gameid);
}

bool addiapp_fields_valid(
    const QXmlStreamReader& xml,
    const QString& lb_dir,
    const QString& rel_path,
    const HashMap<AdditionalAppField, QString, EnumHash>& fields)
{
    const auto id_it = fields.find(AdditionalAppField::ID);
    if (id_it == fields.cend()) {
        log_xml_warning(xml, rel_path, tr_log("additional application has no ID, entry ignored"));
        return false;
    }

    const auto gameid_it = fields.find(AdditionalAppField::GAME_ID);
    if (gameid_it == fields.cend()) {
        log_xml_warning(xml, rel_path, tr_log("additional application has no GameID field, entry ignored"));
        return false;
    }

    const auto path_it = fields.find(AdditionalAppField::PATH);
    if (path_it == fields.cend()) {
        log_xml_warning(xml, rel_path, tr_log("additional application has no path, entry ignored"));
        return false;
    }

    const QFileInfo path_finfo(lb_dir, path_it->second);
    if (!path_finfo.exists()) {
        log_xml_warning(xml, rel_path, tr_log("additional application file `%1` doesn't seem to exist, entry ignored")
            .arg(QDir::toNativeSeparators(path_finfo.absoluteFilePath())));
        return false;
    }

    return true;
}

HashMap<AdditionalAppField, QString, EnumHash> read_addiapp(QXmlStreamReader& xml, const Literals& literals)
{
    HashMap<AdditionalAppField, QString, EnumHash> entries;

    while (xml.readNextStartElement()) {
        const auto field_it = literals.addiappfield_map.find(xml.name().toString());
        if (field_it == literals.addiappfield_map.cend()) {
            xml.skipCurrentElement();
            continue;
        }

        const QString contents = xml.readElementText().trimmed(); // TODO: maybe strrefs
        if (contents.isEmpty())
            continue;

        entries.emplace(field_it->second, contents);
    }

    return entries;
}

void store_game_fields(
    const QString& lb_dir,
    const HashMap<EmulatorId, Emulator>& emulators,
    const HashMap<GameField, QString, EnumHash>& fields,
    modeldata::Game& game)
{
    EmulatorId emu_id;
    QString emu_params;
    QString emu_platform_name;

    for (const auto& pair : fields) {
        switch (pair.first) {
            case GameField::TITLE:
                game.title = pair.second;
                break;
            case GameField::NOTES:
                if (game.description.isEmpty())
                    game.description = pair.second;
                break;
            case GameField::DEVELOPER:
                game.developers.append(pair.second);
                game.developers.removeDuplicates();
                break;
            case GameField::PUBLISHER:
                game.publishers.append(pair.second);
                game.publishers.removeDuplicates();
                break;
            case GameField::GENRE:
                game.genres.append(pair.second);
                game.genres.removeDuplicates();
                break;
            case GameField::RELEASE:
                if (!game.release_date.isValid())
                    game.release_date = QDate::fromString(pair.second, Qt::ISODate);
                break;
            case GameField::STARS:
                if (game.rating < 0.0001f) {
                    bool ok = false;
                    const float fval = pair.second.toFloat(&ok);
                    if (ok && fval > game.rating)
                        game.rating = fval;
                }
                break;
            case GameField::PLAYMODE:
                for (const QStringRef& ref : pair.second.splitRef(QChar(';')))
                    game.genres.append(ref.trimmed().toString());

                game.genres.removeDuplicates();
                break;
            case GameField::EMULATOR_ID:
                emu_id = pair.second;
                break;
            case GameField::EMULATOR_PLATFORM:
                emu_platform_name = pair.second;
                break;
            case GameField::EMULATOR_PARAMS:
                emu_params = pair.second;
                break;
            case GameField::ASSETPATH_VIDEO:
                add_raw_asset_maybe(game.assets, AssetType::VIDEOS, lb_dir, pair.second);
                break;
            case GameField::ASSETPATH_MUSIC:
                add_raw_asset_maybe(game.assets, AssetType::MUSIC, lb_dir, pair.second);
                break;
            case GameField::ID:
            case GameField::PATH:
                // handled earlier
                Q_ASSERT(fields.count(pair.first));
                break;
            default:
                Q_UNREACHABLE();
        }
    }

    if (emu_id.isEmpty()) {
        game.launch_cmd = QStringLiteral("{file.path}");
        game.launch_workdir = QFileInfo(fields.at(GameField::PATH)).absolutePath();
        return;
    }

    const Emulator& emu = emulators.at(emu_id); // checked earlier
    if (emu_params.isEmpty()) {
        emu_params = emu.default_cmd_params;

        // try to use the emulator's platform settings
        if (!emu_platform_name.isEmpty()) {
            const auto emu_platform_it = std::find_if(emu.platforms.cbegin(), emu.platforms.cend(),
                [&emu_platform_name](const EmulatorPlatform& emu_platform){
                    return emu_platform.name == emu_platform_name;
                });
            if (emu_platform_it != emu.platforms.cend())
                emu_params = emu_platform_it->cmd_params;
        }
    }
    game.launch_cmd = QStringLiteral("\"%1\" %2 {file.path}").arg(emu.app_path, emu_params);
    game.launch_workdir = QFileInfo(emu.app_path).absolutePath();
}

size_t store_game(
    const QString& lb_dir,
    const HashMap<EmulatorId, Emulator>& emulators,
    const HashMap<GameField, QString, EnumHash>& fields,
    providers::SearchContext& sctx,
    std::vector<size_t>& collection_childs)
{
    const QFileInfo finfo(lb_dir, fields.at(GameField::PATH));
    const QString can_path = finfo.canonicalFilePath();

    if (!sctx.path_to_gameid.count(can_path)) {
        modeldata::Game game(finfo);

        store_game_fields(lb_dir, emulators, fields, game);
        if (game.launch_cmd.isEmpty())
            qWarning().noquote() << tr_log("LaunchBox: game '%1' has no launch command").arg(game.title);

        const size_t game_id = sctx.games.size();
        sctx.path_to_gameid.emplace(can_path, game_id);
        sctx.games.emplace(game_id, std::move(game));
    }

    const size_t game_id = sctx.path_to_gameid.at(can_path);
    collection_childs.emplace_back(game_id);

    return game_id;
}

bool game_fields_valid(
    const QXmlStreamReader& xml,
    const QString& lb_dir,
    const QString& rel_path,
    const HashMap<EmulatorId, Emulator>& emulators,
    const HashMap<GameField, QString, EnumHash>& fields)
{
    if (fields.find(GameField::ID) == fields.cend()) {
        log_xml_warning(xml, rel_path, tr_log("game has no ID, entry ignored"));
        return false;
    }

    const auto path_it = fields.find(GameField::PATH);
    if (path_it == fields.cend()) {
        log_xml_warning(xml, rel_path, tr_log("game has no path, entry ignored"));
        return false;
    }
    const QFileInfo game_finfo(lb_dir, path_it->second);
    if (!game_finfo.exists()) {
        log_xml_warning(xml, rel_path,
            tr_log("game file `%1` doesn't seem to exist, entry ignored")
                .arg(QDir::toNativeSeparators(game_finfo.absoluteFilePath())));
        return false;
    }

    const auto emu_id_it = fields.find(GameField::EMULATOR_ID);
    if (emu_id_it != fields.cend()) {
        const auto emu_it = emulators.find(emu_id_it->second);
        if (emu_it == emulators.cend()) {
            log_xml_warning(xml, rel_path,
                tr_log("game refers to a missing or invalid emulator with id `%1`, entry ignored").arg(emu_id_it->second));
            return false;
        }

        const Emulator& emu = emu_it->second;
        const auto emu_platform_name_it = fields.find(GameField::EMULATOR_PLATFORM);
        if (emu_platform_name_it != fields.cend()) {
            const QString& emu_platform_name = emu_platform_name_it->second;
            const auto emu_platform_it = std::find_if(emu.platforms.cbegin(), emu.platforms.cend(),
                [&emu_platform_name](const EmulatorPlatform& emu_platform){
                    return emu_platform.name == emu_platform_name;
                });
            if (emu_platform_it == emu.platforms.cend()) {
                log_xml_warning(xml, rel_path,
                    tr_log("game refers to a missing or invalid emulator platform `%1` within emulator `%2`, falling back to emulator defaults")
                        .arg(emu_platform_name, emu.name));
                // not critical, will fall back to default
            }
        }
    }

    return true;
}

HashMap<GameField, QString, EnumHash> read_game(QXmlStreamReader& xml, const Literals& literals)
{
    HashMap<GameField, QString, EnumHash> out;

    while (xml.readNextStartElement()) {
        const auto field_it = literals.gamefield_map.find(xml.name().toString());
        if (field_it == literals.gamefield_map.cend()) {
            xml.skipCurrentElement();
            continue;
        }

        const QString contents = xml.readElementText().trimmed(); // TODO: maybe strrefs
        if (!contents.isEmpty())
            out.emplace(field_it->second, contents); // assume no collisions
    }

    return out;
}
} // namespace launchbox
} // namespace providers


namespace providers {
namespace launchbox {
namespace gamelist_xml {
void read(
    const Provider* const provider,
    const Literals& literals,
    const QString& lb_dir,
    const QString& platform_name,
    const HashMap<EmulatorId, Emulator>& emulators,
    providers::SearchContext& sctx)
{
    const QString xml_rel_path = QLatin1String("Data/Platforms/") % platform_name % QLatin1String(".xml");
    const QString xml_path = lb_dir + xml_rel_path;
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        provider->warn(tr_log("could not open `%1`").arg(QDir::toNativeSeparators(xml_rel_path)));
        return;
    }


    std::vector<size_t>& collection_childs = sctx.collection_childs[platform_name];
    if (sctx.collections.find(platform_name) == sctx.collections.end())
        sctx.collections.emplace(platform_name, modeldata::Collection(platform_name));

    // should be handled after all games have been found
    std::vector<HashMap<AdditionalAppField, QString, EnumHash>> addiapps;
    HashMap<GameId, size_t> gameid_map;


    QXmlStreamReader xml(&xml_file);
    check_lb_root_node(xml, xml_rel_path);

    while (xml.readNextStartElement() && !xml.hasError()) {
        if (xml.name() == QLatin1String("Game")) {
            const HashMap<GameField, QString, EnumHash> fields = read_game(xml, literals);
            if (game_fields_valid(xml, lb_dir, xml_rel_path, emulators, fields)) {
                const size_t gameid = store_game(lb_dir, emulators, fields, sctx, collection_childs);
                gameid_map.emplace(fields.at(GameField::ID), gameid);
            }
            continue;
        }
        if (xml.name() == QLatin1String("AdditionalApplication")) {
            HashMap<AdditionalAppField, QString, EnumHash> fields = read_addiapp(xml, literals);
            if (addiapp_fields_valid(xml, lb_dir, xml_rel_path, fields))
                addiapps.emplace_back(std::move(fields));
            continue;
        }
        xml.skipCurrentElement();
    }


    for (const auto& addiapp_fields : addiapps) {
        const GameId& lb_gameid = addiapp_fields.at(AdditionalAppField::GAME_ID);
        const auto pegasus_gameid_it = gameid_map.find(lb_gameid);

        if (pegasus_gameid_it == gameid_map.cend()) {
            provider->warn(
                tr_log("%1: additional application entry `%2` refers to missing or invalid game `%3`, entry ignored")
                   .arg(QDir::toNativeSeparators(xml_rel_path), addiapp_fields.at(AdditionalAppField::ID), lb_gameid));
            continue;
        }

        store_addiapp(lb_dir, addiapp_fields, pegasus_gameid_it->second, sctx);
    }
}
} // namespace gamelist_xml
} // namespace launchbox
} // namespace providers
