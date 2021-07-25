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

#include "PlayniteMetadataParser.h"

#include "Log.h"
#include "PlayniteComponents.h"
#include "PlayniteJsonHelper.h"

#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>

namespace providers {
namespace playnite {

PlayniteMetadataParser::PlayniteMetadataParser(QString log_tag, const QDir& playnite_dir)
    : m_log_tag(std::move(log_tag))
    , m_playnite_dir(playnite_dir)
    , m_dir_filters(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot)
    , m_json_ext_list(QStringList() << QStringLiteral("*.json"))
{}

PlayniteComponents PlayniteMetadataParser::parse_metadata() const
{
    PlayniteComponents components;
    components.platforms = parse_platform_metadata();
    components.sources = parse_source_metadata();
    components.companies = parse_company_metadata();
    components.genres = parse_genre_metadata();
    components.emulators = parse_emulator_metadata();
    components.games = parse_game_metadata();
    return components;
}

HashMap<QString, QString> PlayniteMetadataParser::parse_platform_metadata() const
{
    return parse_id_name_files(QStringLiteral("library/platforms"));
}

HashMap<QString, QString> PlayniteMetadataParser::parse_source_metadata() const
{
    return parse_id_name_files(QStringLiteral("library/sources"));
}

HashMap<QString, QString> PlayniteMetadataParser::parse_company_metadata() const
{
    return parse_id_name_files(QStringLiteral("library/companies"));
}

HashMap<QString, QString> PlayniteMetadataParser::parse_genre_metadata() const
{
    return parse_id_name_files(QStringLiteral("library/genres"));
}

HashMap<QString, PlayniteEmulator> PlayniteMetadataParser::parse_emulator_metadata() const
{
    HashMap<QString, PlayniteEmulator> emulator_map;
    QDirIterator dir_it(m_playnite_dir.filePath(QStringLiteral("library/emulators/")), m_json_ext_list, m_dir_filters);
    while (dir_it.hasNext()) {
        const QString file_path = dir_it.next();
        const QJsonObject json_object = get_json_object_from_file(file_path);
        if (json_object.isEmpty()) {
            Log::info(m_log_tag, LOGMSG("Skipping missing JSON file %1").arg(file_path));
            continue;
        }

        PlayniteEmulator emu;
        const auto emu_json = JsonObjectHelper(json_object);
        emu.id = emu_json.get_string(QStringLiteral("Id"));
        emu.name = emu_json.get_string(QStringLiteral("name"));
        auto profiles = emu_json.get_json_array(QStringLiteral("Profiles"));
        for (const QJsonValueRef& profile : profiles) {
            PlayniteEmulator::EmulatorProfile emu_profile;
            auto profile_json = JsonObjectHelper(profile.toObject());
            emu_profile.executable = profile_json.get_string(QStringLiteral("Executable"));
            emu_profile.arguments = profile_json.get_string(QStringLiteral("Arguments"));
            emu_profile.working_dir = profile_json.get_string(QStringLiteral("WorkingDirectory"));
            emu_profile.id = profile_json.get_string(QStringLiteral("Id"));
            emu_profile.name = profile_json.get_string(QStringLiteral("Name"));
            emu.profiles.emplace(emu_profile.id, emu_profile);
        }
        emulator_map.emplace(emu.id, emu);

    }
    return emulator_map;
}

std::vector<PlayniteGame> PlayniteMetadataParser::parse_game_metadata() const
{
    std::vector<PlayniteGame> output;
    QDirIterator dir_it(m_playnite_dir.filePath(QStringLiteral("library/games/")), m_json_ext_list, m_dir_filters);
    while (dir_it.hasNext()) {
        const QString file_path = dir_it.next();
        const QJsonObject json_object = get_json_object_from_file(file_path);
        if (json_object.isEmpty()) {
            Log::info(m_log_tag, LOGMSG("Skipping missing JSON file %1").arg(file_path));
            continue;
        }

        PlayniteGame game;
        const auto game_json = JsonObjectHelper(json_object);
        game.name = game_json.get_string(QStringLiteral("Name"));
        game.background_image = game_json.get_string(QStringLiteral("BackgroundImage"));
        game.community_score = game_json.get_float(QStringLiteral("CommunityScore"));
        game.cover_image = game_json.get_string(QStringLiteral("CoverImage"));
        game.description = game_json.get_string(QStringLiteral("Description"));
        game.developer_ids = game_json.get_string_list(QStringLiteral("DeveloperIds"));
        game.game_id = game_json.get_string(QStringLiteral("GameId"));
        game.genre_ids = game_json.get_string_list(QStringLiteral("GenreIds"));
        game.id = game_json.get_string(QStringLiteral("Id"));
        game.platform_id = game_json.get_string(QStringLiteral("PlatformId"));
        game.publisher_ids = game_json.get_string_list(QStringLiteral("PublisherIds"));
        game.release_date = game_json.get_string(QStringLiteral("ReleaseDate"));
        game.installed = game_json.get_bool(QStringLiteral("IsInstalled"));
        game.hidden = game_json.get_bool(QStringLiteral("Hidden"));
        game.source_id = game_json.get_string(QStringLiteral("SourceId"));
        game.install_directory = game_json.get_string(QStringLiteral("InstallDirectory"));
        game.game_image_path = game_json.get_string(QStringLiteral("GameImagePath"));

        PlayniteGame::PlayAction action;
        const auto play_action_json = game_json.get_json_object_helper(QStringLiteral("PlayAction"));
        action.arguments = play_action_json.get_string(QStringLiteral("Arguments"));
        action.path = play_action_json.get_string(QStringLiteral("Path"));
        action.working_dir = play_action_json.get_string(QStringLiteral("WorkingDir"));
        action.emulator_id = play_action_json.get_string(QStringLiteral("EmulatorId"));
        action.emulator_profile_id = play_action_json.get_string(QStringLiteral("EmulatorProfileId"));
        action.type = play_action_json.get_int(QStringLiteral("Type"));
        game.play_action = action;
        output.emplace_back(std::move(game));
    }
    return output;
}

HashMap<QString, QString> PlayniteMetadataParser::parse_id_name_files(const QString& rel_path) const
{
    HashMap<QString, QString> name_map;
    QDirIterator dir_it(m_playnite_dir.filePath(rel_path), m_json_ext_list, m_dir_filters);
    while (dir_it.hasNext()) {
        const QString file_path = dir_it.next();
        const QJsonObject json_object = get_json_object_from_file(file_path);
        if (json_object.isEmpty()) {
            Log::info(m_log_tag, LOGMSG("Skipping missing JSON file %1").arg(file_path));
            continue;
        }

        const auto json = JsonObjectHelper(json_object);
        const QString name = json.get_string(QStringLiteral("Name"));
        const QString id = json.get_string(QStringLiteral("Id"));
        name_map.emplace(id, name);
    }
    return name_map;
}

QJsonObject PlayniteMetadataParser::get_json_object_from_file(const QString& file_path) const
{
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Log::info(m_log_tag, LOGMSG("Could not open %1").arg(QDir::toNativeSeparators(file_path)));
        return {};
    }

    QJsonParseError json_error{};
    const QJsonDocument json_doc = QJsonDocument::fromJson(file.readAll(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        Log::info(m_log_tag, LOGMSG("Invalid JSON file at %1").arg(file_path));
        return {};
    }
    return json_doc.object();
}

} // namespace playnite
} // namespace providers
