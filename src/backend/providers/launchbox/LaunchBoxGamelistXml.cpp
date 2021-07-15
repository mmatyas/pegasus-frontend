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


#include "LaunchBoxGamelistXml.h"

#include "Log.h"
#include "model/gaming/Assets.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "providers/SearchContext.h"
#include "providers/launchbox/LaunchBoxXml.h"
#include "providers/steam/SteamProvider.h"
#include "utils/PathTools.h"

#include <QDir>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <array>


namespace providers {
namespace launchbox {

enum class GameField : unsigned char {
    ID,
    PATH,
    TITLE,
    SORT_TITLE,
    RELEASE,
    DEVELOPER,
    PUBLISHER,
    NOTES,
    PLAYMODE,
    GENRE,
    STARS,
    EMULATOR_ID,
    EMULATOR_PARAMS,
    EMULATOR_PLATFORM,
    ASSETPATH_VIDEO,
    ASSETPATH_MUSIC,
    SOURCE,
};
enum class AppField : unsigned char {
    ID,
    GAME_ID,
    PATH,
    NAME,
};

void apply_game_fields(
    const HashMap<GameField, QString>& fields,
    model::Game& game,
    const HashMap<QString, Emulator>& emulators)
{
    const QString steam_call(::providers::steam::find_steam_call());

    QString emu_id;
    QString emu_params;
    QString emu_platform_name;
    QString source;
    QString path;

    // TODO: C++17
    for (const auto& pair : fields) {
        switch (pair.first) {
            case GameField::TITLE:
                game.setTitle(pair.second);
                break;
            case GameField::SORT_TITLE:
                game.setSortBy(pair.second);
                break;
            case GameField::NOTES:
                if (game.description().isEmpty())
                    game.setDescription(pair.second);
                if (game.summary().isEmpty())
                    game.setSummary(pair.second);
                break;
            case GameField::DEVELOPER:
                game.developerList().append(pair.second);
                break;
            case GameField::PUBLISHER:
                game.publisherList().append(pair.second);
                break;
            case GameField::GENRE:
                game.genreList().append(pair.second);
                break;
            case GameField::RELEASE:
                if (!game.releaseDate().isValid())
                    game.setReleaseDate(QDate::fromString(pair.second, Qt::ISODate));
                break;
            case GameField::STARS:
                if (game.rating() < 0.0001f) {
                    bool ok = false;
                    const float fval = pair.second.toFloat(&ok);
                    if (ok && fval > game.rating())
                        game.setRating(fval);
                }
                break;
            case GameField::PLAYMODE:
                for (const QStringRef& ref : pair.second.splitRef(QChar(';')))
                    game.genreList().append(ref.trimmed().toString());
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
                game.assetsMut().add_file(AssetType::VIDEO, pair.second);
                break;
            case GameField::ASSETPATH_MUSIC:
                game.assetsMut().add_file(AssetType::MUSIC, pair.second);
                break;
            case GameField::SOURCE:
                source = pair.second;
                break;
            case GameField::PATH:
                path = pair.second;
                break;
            case GameField::ID:
                // handled earlier or handled dfferently
                break;
        }
    }

    if (emu_id.isEmpty()) {
        if (source == QLatin1String("Steam")) {
            game.setLaunchCmd(steam_call % path);
        } else {
            game.setLaunchCmd(QStringLiteral("{file.path}"));
            game.setLaunchWorkdir(::clean_abs_dir(QFileInfo(path)));
        }
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
            if (emu_platform_it != emu.platforms.cend()) {
                if (!emu_platform_it->cmd_params.isEmpty())
                    emu_params = emu_platform_it->cmd_params;
            }
        }
    }
    game.setLaunchCmd(QStringLiteral("\"%1\" %2 {file.path}").arg(emu.app_path, emu_params));
    game.setLaunchWorkdir(::clean_abs_dir(QFileInfo(emu.app_path)));
}

void apply_app_fields(const HashMap<AppField, QString>& fields, model::GameFile& entry)
{
    const auto name_it = fields.find(AppField::NAME);
    if (name_it != fields.cend())
        entry.setName(name_it->second);
}

} // namespace launchbox
} // namespace providers


namespace providers {
namespace launchbox {

GamelistXml::GamelistXml(QString log_tag, QDir lb_root)
    : m_log_tag(std::move(log_tag))
    , m_lb_root(std::move(lb_root))
    , m_game_keys {
        { QStringLiteral("ID"), GameField::ID },
        { QStringLiteral("ApplicationPath"), GameField::PATH },
        { QStringLiteral("Title"), GameField::TITLE },
        { QStringLiteral("SortTitle"), GameField::SORT_TITLE },
        { QStringLiteral("Developer"), GameField::DEVELOPER },
        { QStringLiteral("Publisher"), GameField::PUBLISHER },
        { QStringLiteral("ReleaseDate"), GameField::RELEASE },
        { QStringLiteral("Notes"), GameField::NOTES },
        { QStringLiteral("PlayMode"), GameField::PLAYMODE },
        { QStringLiteral("Genre"), GameField::GENRE },
        { QStringLiteral("CommunityStarRating"), GameField::STARS },
        { QStringLiteral("Emulator"), GameField::EMULATOR_ID },
        { QStringLiteral("CommandLine"), GameField::EMULATOR_PARAMS },
        { QStringLiteral("Platform"), GameField::EMULATOR_PLATFORM },
        { QStringLiteral("VideoPath"), GameField::ASSETPATH_VIDEO },
        { QStringLiteral("MusicPath"), GameField::ASSETPATH_MUSIC },
        { QStringLiteral("Source"), GameField::SOURCE },
    }
    , m_app_keys {
        { QStringLiteral("Id"), AppField::ID },
        { QStringLiteral("ApplicationPath"), AppField::PATH },
        { QStringLiteral("GameID"), AppField::GAME_ID },
        { QStringLiteral("Name"), AppField::NAME },
    }
    , m_rx_steam_uri(QStringLiteral("^steam://rungameid/(\\d+)$"))
{}

void GamelistXml::log_xml_warning(const QString& xml_path, const size_t linenum, const QString& msg) const
{
    Log::warning(m_log_tag, LOGMSG("In `%1` at line %2: %3")
        .arg(QDir::toNativeSeparators(xml_path), QString::number(linenum), msg));
}

bool GamelistXml::game_fields_valid(
    const QString& xml_path,
    const size_t xml_linenum,
    const HashMap<GameField, QString>& fields,
    const HashMap<QString, Emulator>& emulators) const
{
    if (fields.find(GameField::ID) == fields.cend()) {
        log_xml_warning(xml_path, xml_linenum, LOGMSG("Game has no ID, entry ignored"));
        return false;
    }

    const auto path_it = fields.find(GameField::PATH);
    if (path_it == fields.cend()) {
        log_xml_warning(xml_path, xml_linenum, LOGMSG("Game has no path, entry ignored"));
        return false;
    }

    const auto emu_id_it = fields.find(GameField::EMULATOR_ID);
    if (emu_id_it != fields.cend()) {
        const auto emu_it = emulators.find(emu_id_it->second);
        if (emu_it == emulators.cend()) {
            log_xml_warning(xml_path, xml_linenum,
                LOGMSG("Game refers to a missing or invalid emulator with id `%1`, entry ignored").arg(emu_id_it->second));
            return false;
        }

        const auto emu_platform_name_it = fields.find(GameField::EMULATOR_PLATFORM);
        if (emu_platform_name_it != fields.cend()) {
            const Emulator& emu = emu_it->second;

            const QString& emu_platform_name = emu_platform_name_it->second;
            const auto emu_platform_it = std::find_if(emu.platforms.cbegin(), emu.platforms.cend(),
                [&emu_platform_name](const EmulatorPlatform& emu_platform){
                    return emu_platform.name == emu_platform_name;
                });
            if (emu_platform_it == emu.platforms.cend()) {
                log_xml_warning(xml_path, xml_linenum,
                    LOGMSG("Game refers to a missing or invalid emulator platform `%1` within emulator `%2`, falling back to emulator defaults")
                        .arg(emu_platform_name, emu.name));
                // not critical, will fall back to default
            }
        }
    }

    return true;
}

bool GamelistXml::app_fields_valid(
    const QString& xml_path,
    const size_t xml_linenum,
    const HashMap<AppField, QString>& fields) const
{
    const auto id_it = fields.find(AppField::ID);
    if (id_it == fields.cend()) {
        log_xml_warning(xml_path, xml_linenum, LOGMSG("Additional application has no ID, entry ignored"));
        return false;
    }

    const auto gameid_it = fields.find(AppField::GAME_ID);
    if (gameid_it == fields.cend()) {
        log_xml_warning(xml_path, xml_linenum, LOGMSG("Additional application has no GameID field, entry ignored"));
        return false;
    }

    const auto path_it = fields.find(AppField::PATH);
    if (path_it == fields.cend()) {
        log_xml_warning(xml_path, xml_linenum, LOGMSG("Additional application has no path, entry ignored"));
        return false;
    }

    if (!QFileInfo::exists(path_it->second)) {
        log_xml_warning(xml_path, xml_linenum, LOGMSG("Additional application file `%1` doesn't seem to exist, entry ignored")
            .arg(QDir::toNativeSeparators(path_it->second)));
        return false;
    }

    return true;
}

HashMap<GameField, QString> GamelistXml::read_game_node(QXmlStreamReader& xml) const
{
    HashMap<GameField, QString> fields;

    while (xml.readNextStartElement()) {
        const auto field_it = m_game_keys.find(xml.name().toString());
        if (field_it == m_game_keys.cend()) {
            xml.skipCurrentElement();
            continue;
        }

        QString contents = xml.readElementText().trimmed();
        if (!contents.isEmpty())
            fields.emplace(field_it->second, std::move(contents));
    }

    // TODO: C++17 Class template argument deduction
    constexpr std::array<GameField, 2> ABSPATH_KEYS {
        GameField::ASSETPATH_VIDEO,
        GameField::ASSETPATH_MUSIC,
    };
    for (const GameField key : ABSPATH_KEYS) {
        const auto it = fields.find(key);
        if (it != fields.cend())
            it->second = ::clean_abs_path(QFileInfo(m_lb_root, it->second));
    }

    return fields;
}

HashMap<AppField, QString> GamelistXml::read_app_node(QXmlStreamReader& xml) const
{
    HashMap<AppField, QString> fields;

    while (xml.readNextStartElement()) {
        const auto field_it = m_app_keys.find(xml.name().toString());
        if (field_it == m_app_keys.cend()) {
            xml.skipCurrentElement();
            continue;
        }

        QString contents = xml.readElementText().trimmed();
        if (!contents.isEmpty())
            fields.emplace(field_it->second, std::move(contents));
    }


    constexpr std::array<AppField, 1> PATH_KEYS {
        AppField::PATH,
    };
    for (const AppField key : PATH_KEYS) {
        const auto it = fields.find(key);
        if (it != fields.cend())
            it->second = ::clean_abs_path(QFileInfo(m_lb_root, it->second));
    }


    return fields;
}

std::vector<model::Game*> GamelistXml::find_games_for(
    const Platform& platform,
    const HashMap<QString, Emulator>& emulators,
    SearchContext& sctx) const
{
    const QString xml_rel_path = QStringLiteral("Data/Platforms/%1.xml").arg(platform.name); // TODO: Qt 5.14+ QLatin1String
    const QString xml_path = m_lb_root.filePath(xml_rel_path);

    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        Log::error(m_log_tag, LOGMSG("Could not open `%1`").arg(QDir::toNativeSeparators(xml_rel_path)));
        return {};
    }


    QXmlStreamReader xml(&xml_file);
    verify_root_node(xml);

    model::Collection& collection = *sctx.get_or_create_collection(platform.name);
    collection.setSortBy(platform.sort_by);

    // should be handled after all games have been found
    std::vector<HashMap<AppField, QString>> addiapps;
    HashMap<QString, model::Game*> gameid_map;

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("Game")) {
            const size_t linenum = xml.lineNumber();

            const HashMap<GameField, QString> fields = read_game_node(xml);
            const bool node_valid = game_fields_valid(xml_path, linenum, fields, emulators);
            if (!node_valid)
                continue;

            Q_ASSERT(fields.count(GameField::PATH));
            Q_ASSERT(fields.count(GameField::ID));

            const QString& game_path = fields.at(GameField::PATH);
            model::Game* game_ptr = nullptr;

            const auto source_it = fields.find(GameField::SOURCE);
            const QString source = (source_it != fields.cend())
                ? source_it->second
                : QString();
            if (source == QLatin1String("Steam")) {
                const auto match = m_rx_steam_uri.match(game_path);
                if (!match.hasMatch()) {
                    log_xml_warning(xml_path, linenum, LOGMSG("Game was expected to be a Steam game, but its path field seems to be incorrect"));
                    continue;
                }

                const QString steam_id = match.captured(1);
                const QString steam_uri = QStringLiteral("steam:") + steam_id;

                game_ptr = sctx.game_by_uri(steam_uri);
                if (!game_ptr) {
                    game_ptr = sctx.create_game_for(collection);
                    sctx.game_add_uri(*game_ptr, steam_uri);
                }
            }
            else {
                const QFileInfo finfo(m_lb_root, game_path);
                if (!finfo.exists()) {
                    log_xml_warning(xml_path, linenum, LOGMSG("Game file `%1` doesn't seem to exist, entry ignored").arg(QDir::toNativeSeparators(game_path)));
                    continue;
                }

                QString abs_path = ::clean_abs_path(finfo);
                game_ptr = sctx.game_by_filepath(abs_path);
                if (!game_ptr) {
                    game_ptr = sctx.create_game_for(collection);
                    sctx.game_add_filepath(*game_ptr, std::move(abs_path));
                }
            }

            Q_ASSERT(game_ptr);
            apply_game_fields(fields, *game_ptr, emulators);
            gameid_map.emplace(fields.at(GameField::ID), game_ptr);
            sctx.game_add_to(*game_ptr, collection);
            continue;
        }

        if (xml.name() == QLatin1String("AdditionalApplication")) {
            const size_t linenum = xml.lineNumber();

            HashMap<AppField, QString> fields = read_app_node(xml);
            if (app_fields_valid(xml_path, linenum, fields))
                addiapps.emplace_back(std::move(fields));

            continue;
        }

        xml.skipCurrentElement();
    }
    if (xml.error())
        Log::error(m_log_tag, LOGMSG("`%1`: %2").arg(xml_path, xml.errorString()));


    for (const HashMap<AppField, QString>& fields : addiapps) {
        Q_ASSERT(fields.count(AppField::ID));
        Q_ASSERT(fields.count(AppField::GAME_ID));
        Q_ASSERT(fields.count(AppField::PATH));

        const QString& game_id = fields.at(AppField::GAME_ID);
        const auto it = gameid_map.find(game_id);
        if (it == gameid_map.cend()) {
            const QString app_id = fields.at(AppField::ID);
            Log::warning(m_log_tag, LOGMSG("In `%1` additional application entry `%2` refers to missing or invalid game `%3`, entry ignored")
                .arg(QDir::toNativeSeparators(xml_path), app_id, game_id));
            continue;
        }

        const QString abs_path = ::clean_abs_path(QFileInfo(fields.at(AppField::PATH)));
        Q_ASSERT(!abs_path.isEmpty());

        model::Game& game = *(it->second);
        model::GameFile* entry_ptr = sctx.gamefile_by_filepath(abs_path);
        if (!entry_ptr)
            entry_ptr = sctx.game_add_filepath(game, abs_path);

        apply_app_fields(fields, *entry_ptr);
    }


    std::vector<model::Game*> found_games;
    found_games.reserve(gameid_map.size());
    // TODO: C++17
    for (const auto& pair : gameid_map)
        found_games.emplace_back(pair.second);

    return found_games;
}

} // namespace launchbox
} // namespace providers
