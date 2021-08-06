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

#include "PlayniteProvider.h"

#include "Log.h"
#include "Paths.h"
#include "PlayniteComponents.h"
#include "PlayniteLaunchHelper.h"
#include "PlayniteMetadataParser.h"
#include "model/gaming/Assets.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"
#include "utils/PathTools.h"

#include <QTextDocument>

namespace {
QString default_installation()
{
    return paths::homePath() + QStringLiteral("/AppData/Roaming/Playnite/");
}
} // namespace

namespace providers {
namespace playnite {

namespace {
model::Game* create_file_path_game(const QString& path, model::Collection& collection, SearchContext& sctx)
{
    auto game_ptr = sctx.game_by_filepath(path);
    if (!game_ptr) {
        game_ptr = sctx.create_game_for(collection);
        sctx.game_add_filepath(*game_ptr, path);
    }
    return game_ptr;
}

model::Game* create_uri_game(const QString& key, model::Collection& collection, SearchContext& sctx)
{
    auto game_ptr = sctx.game_by_uri(key);
    if (!game_ptr) {
        game_ptr = sctx.create_game_for(collection);
        sctx.game_add_uri(*game_ptr, key);
    }
    return game_ptr;
}

void apply_game_fields(
    model::Game& game,
    const PlayniteComponents& components,
    const PlayniteGame& game_info,
    const QDir& playnite_dir)
{
    game.setTitle(game_info.name);

    QTextDocument doc;
    doc.setHtml(game_info.description);
    const QString description = doc.toPlainText();

    if (game.description().isEmpty())
        game.setDescription(description);

    if (game.summary().isEmpty())
        game.setSummary(description);

    for (const QString& developer_id : game_info.developer_ids) {
        auto developer_it = components.companies.find(developer_id);
        if (developer_it != components.companies.end()) {
            game.developerList().append(developer_it->second);
        }
    }

    for (const QString& publisher_id : game_info.publisher_ids) {
        auto publisher_it = components.companies.find(publisher_id);
        if (publisher_it != components.companies.end()) {
            game.publisherList().append(publisher_it->second);
        }
    }

    for (const QString& genre_id : game_info.genre_ids) {
        auto genre_it = components.genres.find(genre_id);
        if (genre_it != components.genres.end()) {
            game.genreList().append(genre_it->second);
        }
    }

    if (!game.releaseDate().isValid())
        game.setReleaseDate(QDate::fromString(game_info.release_date, Qt::ISODate));

    game.setRating(game_info.community_score / 100.0f);

    const QString playnite_dir_path = playnite_dir.absolutePath();
    if (!game_info.cover_image.isEmpty()) {
        const QString cover_path = QStringLiteral("%1/library/files/%2").arg(playnite_dir_path, game_info.cover_image);
        game.assetsMut().add_file(AssetType::BOX_FRONT, cover_path);
    }

    if (!game_info.background_image.isEmpty()) {
        const QString bg_path =
            QStringLiteral("%1/library/files/%2").arg(playnite_dir_path, game_info.background_image);
        game.assetsMut().add_file(AssetType::BACKGROUND, bg_path);
    }

    const QString extra_metadata_path = QStringLiteral("%1/ExtraMetadata/games/%2").arg(playnite_dir_path, game_info.id);
    const QFileInfo logo_file(extra_metadata_path + QStringLiteral("/Logo.png"));
    if (logo_file.exists() && logo_file.isFile()) {
        game.assetsMut().add_file(AssetType::LOGO, logo_file.absoluteFilePath());
    }

    const QFileInfo video_trailer(extra_metadata_path + QStringLiteral("/VideoTrailer.mp4"));
    if (video_trailer.exists() && video_trailer.isFile()) {
        game.assetsMut().add_file(AssetType::VIDEO, video_trailer.absoluteFilePath());
    }
}
} // namespace

PlayniteProvider::PlayniteProvider(QObject* parent)
    : Provider(QLatin1String("playnite"), QStringLiteral("Playnite"), parent)
{}

Provider& PlayniteProvider::run(SearchContext& sctx)
{
    const QString playnite_dir_path = [this] {
        const auto option_it = options().find(QStringLiteral("installdir"));
        return (option_it != options().cend())
            ? QDir::cleanPath(option_it->second.front()) + QLatin1Char('/')
            : default_installation();
    }();

    if (playnite_dir_path.isEmpty() || !QFileInfo::exists(playnite_dir_path)) {
        Log::info(display_name(), LOGMSG("No installation found"));
        return *this;
    }

    Log::info(display_name(), LOGMSG("Looking for installation at `%1`").arg(QDir::toNativeSeparators(playnite_dir_path)));

    const std::vector<model::Game*> games = create_games(playnite_dir_path, sctx);

    if (games.empty()) {
        Log::warning(display_name(), LOGMSG("No games found"));
    }
    return *this;
}

std::vector<model::Game*> PlayniteProvider::create_games(const QString& playnite_path, SearchContext& sctx)
{
    const QDir playnite_dir(playnite_path);
    const PlayniteMetadataParser playnite_metadata_parser(display_name(), playnite_path);
    const PlayniteComponents& components = playnite_metadata_parser.parse_metadata();

    std::vector<model::Game*> output;
    PlayniteLaunchHelper launch_helper;

    const float num_progress_updates = 5;
    int update_interval = components.games.size() / num_progress_updates;
    if (update_interval == 0)
        update_interval = 1;
    const float progress_step = 1.f / num_progress_updates;
    int game_count = 0;
    float progress = 0.f;

    for (const PlayniteGame& game_info : components.games) {
        game_count++;
        progress += progress_step;
        const bool should_update_progress = game_count % update_interval == 0;
        if (!validate_game(game_info, components)) {
            if (should_update_progress)
                emit progressChanged(progress);
            continue;
        }

        // By default, Playnite categorizes Windows games as "PC", which is handled by Pegasus default theme using the
        // IBM logo. For a good out-of-the-box experience, this should be "Windows".
        QString platform_name = components.platforms.at(game_info.platform_id);
        if (platform_name == QLatin1String("PC"))
            platform_name = QStringLiteral("Windows");
        model::Collection& collection = *sctx.get_or_create_collection(platform_name);

        PlayniteGameLaunchInfo launch_info = launch_helper.get_game_launch_info(game_info, components);

        model::Game* game_ptr = nullptr;
        if (launch_info.type == PlayniteGameLaunchInfo::LaunchType::STEAM) {
            QString key = QStringLiteral("steam:%1").arg(game_info.game_id);
            game_ptr = create_uri_game(key, collection, sctx);
        }
        else {
            const QFileInfo finfo(launch_info.path);
            game_ptr = create_file_path_game(clean_abs_path(finfo), collection, sctx);
        }

        Q_ASSERT(game_ptr);

        game_ptr->setLaunchCmd(launch_info.launch_cmd);
        game_ptr->setLaunchWorkdir(launch_info.working_dir);
        apply_game_fields(*game_ptr, components, game_info, playnite_dir);
        sctx.game_add_to(*game_ptr, collection);
        output.emplace_back(game_ptr);

        if (should_update_progress)
            emit progressChanged(progress);
    }

    return output;
}

bool PlayniteProvider::validate_game(const PlayniteGame& game, const PlayniteComponents& components) const
{
    if (game.name.isEmpty()) {
        return false;
    }

    if (game.id.isEmpty()) {
        Log::info(display_name(), LOGMSG("No id found for game %1, skipping...").arg(game.name));
        return false;
    }

    if (!game.installed) {
        Log::info(display_name(), LOGMSG("Game %1 is not installed, skipping...").arg(game.name));
        return false;
    }

    if (game.hidden) {
        Log::info(display_name(), LOGMSG("Game %1 is hidden, skipping...").arg(game.name));
        return false;
    }

    const auto platform_name_it = components.platforms.find(game.platform_id);
    if (platform_name_it == components.platforms.end()) {
        Log::info(display_name(), LOGMSG("No platform found for game %1, skipping...").arg(game.name));
        return false;
    }

    const PlayniteGameLaunchInfo::LaunchType launch_type = PlayniteLaunchHelper::get_launch_type_enum(game.play_action.type);
    if (launch_type == PlayniteGameLaunchInfo::LaunchType::INSTALLED) {
        // Installed
        if (game.play_action.path.isEmpty()) {
            Log::info(display_name(), LOGMSG("No path found for game %1, skipping...").arg(game.name));
            return false;
        }

        if (game.install_directory.isEmpty()) {
            Log::info(display_name(), LOGMSG("No install directory found for game %1, skipping...").arg(game.name));
            return false;
        }
    }
    else if (launch_type == PlayniteGameLaunchInfo::LaunchType::STEAM) {
        // Launcher
        const auto source_it = components.sources.find(game.source_id);
        if (source_it == components.sources.end()) {
            Log::info(display_name(), LOGMSG("No source found for game %1, skipping...").arg(game.name));
            return false;
        }

        QString source = source_it->second;
        if (source != "Steam") {
            Log::info(display_name(), LOGMSG("Unsupported source %1 for game %2, skipping...").arg(source, game.name));
            return false;
        }

        if (game.play_action.path.isEmpty()) {
            Log::info(display_name(), LOGMSG("No launch url found for game %1, skipping...").arg(game.name));
            return false;
        }

        if (game.game_id.isEmpty()) {
            Log::info(display_name(), LOGMSG("No game id for game %1, skipping...").arg(game.name));
            return false;
        }
    }
    else if (launch_type == PlayniteGameLaunchInfo::LaunchType::EMULATOR) {
        // Emulator
        if (game.play_action.emulator_id.isEmpty()) {
            Log::info(display_name(), LOGMSG("No emulator id found for game %1, skipping...").arg(game.name));
            return false;
        }

        if (game.game_image_path.isEmpty()) {
            Log::info(display_name(), LOGMSG("No game image path found for game %1, skipping...").arg(game.name));
            return false;
        }

        const auto emu_it = components.emulators.find(game.play_action.emulator_id);
        if (emu_it == components.emulators.end()) {
            Log::info(display_name(), LOGMSG("No emulator found for game %1, skipping...").arg(game.name));
            return false;
        }

        const PlayniteEmulator& emu = emu_it->second;
        const auto& emu_profile_it = emu.profiles.find(game.play_action.emulator_profile_id);
        if (emu_profile_it == emu.profiles.end()) {
            Log::info(display_name(), LOGMSG("No emulator profile found for game %1, skipping...").arg(game.name));
            return false;
        }

        const PlayniteEmulator::EmulatorProfile& emulator_profile = emu_profile_it->second;
        if (emulator_profile.executable.isEmpty()) {
            Log::info(display_name(), LOGMSG("No emulator path found for game %1, skipping...").arg(game.name));
            return false;
        }
    }
    else {
        Log::info(display_name(), LOGMSG("Unknown play action type %1 for game %2, skipping...").arg(QString::number(game.play_action.type), game.name));
        return false;
    }
    return true;
}

} // namespace playnite
} // namespace providers
