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


#include "LaunchboxProvider.h"

#include "Paths.h"
#include "LocaleUtils.h"
#include "types/AssetType.h"
#include "utils/StdHelpers.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QXmlStreamReader>
#include <unordered_set>


namespace {
static constexpr auto MSG_PREFIX = "LaunchBox:";

enum class GameField : unsigned char {
    PATH,
    TITLE,
    RELEASE,
    DEVELOPER,
    PUBLISHER,
    NOTES,
    PLAYMODE,
    GENRE,
    STARS,
    EMULATOR,
    EMULATOR_PARAMS,
};

using EmulatorId = QString;
struct Emulator {
    QString app_path;
    QString cmd_params;

    bool incomplete() const { return app_path.isEmpty(); }
    MOVE_ONLY(Emulator)
};
struct Platform {
    EmulatorId default_emu_id;
    QString name;
    QString cmd_params;
    QString xml_path;

    bool incomplete() const {
        return default_emu_id.isEmpty() || name.isEmpty() || xml_path.isEmpty();
    }
    MOVE_ONLY(Platform)
};


HashMap<QString, const size_t>
build_title_map(const std::vector<size_t>& coll_childs, const HashMap<size_t, modeldata::Game>& games)
{
    const QRegularExpression rx_invalid(QStringLiteral(R"([<>:"\/\\|?*'])"));
    const QString underscore(QLatin1Char('_'));

    HashMap<QString, const size_t> out;
    for (const size_t gameid : coll_childs) {
        QString title = games.at(gameid).title;
        title.replace(rx_invalid, underscore);
        out.emplace(std::move(title), gameid);
    }
    return out;
}

void find_assets_in(const QString& asset_dir,
                    const AssetType asset_type,
                    const HashMap<QString, const size_t>& title_to_gameid_map,
                    HashMap<size_t, modeldata::Game>& games)
{
    constexpr auto files_only = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto recursive = QDirIterator::Subdirectories;

    QDirIterator file_it(asset_dir, files_only, recursive);
    while (file_it.hasNext()) {
        file_it.next();

        const QString basename = file_it.fileInfo().completeBaseName();
        const QString game_title = basename.left(basename.length() - 3); // gamename "-xx" .ext

        const auto it = title_to_gameid_map.find(game_title);
        if (it == title_to_gameid_map.cend())
            continue;

        modeldata::Game& game = games.at(it->second);
        game.assets.addFileMaybe(asset_type, file_it.filePath());
    }
}

void find_assets(const QString& lb_dir, const Platform& platform,
                 const std::vector<std::pair<QString, AssetType>>& assetdir_map,
                 providers::SearchContext& sctx)
{
    const auto coll_childs_it = sctx.collection_childs.find(platform.name);
    if (coll_childs_it == sctx.collection_childs.cend())
        return;

    const std::vector<size_t>& collection_childs = coll_childs_it->second;
    const HashMap<QString, const size_t> title_to_gameid_map = build_title_map(collection_childs, sctx.games);


    const QString images_root = lb_dir % QLatin1String("Images/") % platform.name % QLatin1Char('/');
    for (const auto& assetdir_pair : assetdir_map) {
        const QString assetdir_path = images_root + assetdir_pair.first;
        const AssetType assetdir_type = assetdir_pair.second;
        find_assets_in(assetdir_path, assetdir_type, title_to_gameid_map, sctx.games);
    }

    const QString music_root = lb_dir % QLatin1String("Music/") % platform.name % QLatin1Char('/');
    find_assets_in(music_root, AssetType::MUSIC, title_to_gameid_map, sctx.games);

    const QString video_root = lb_dir % QLatin1String("Videos/") % platform.name % QLatin1Char('/');
    find_assets_in(video_root, AssetType::VIDEOS, title_to_gameid_map, sctx.games);
}

void store_game_fields(modeldata::Game& game, const HashMap<GameField, QString, EnumHash>& fields,
                       const Platform& platform, const HashMap<EmulatorId, Emulator>& emulators)
{
    QString emu_app = emulators.at(platform.default_emu_id).app_path;
    QString emu_params = platform.cmd_params.isEmpty()
        ? emulators.at(platform.default_emu_id).app_path
        : platform.cmd_params;

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
            case GameField::EMULATOR: {
                const auto emu_it = emulators.find((pair.second));
                if (emu_it != emulators.cend())
                    emu_app = emu_it->second.app_path;
                break;
            }
            case GameField::EMULATOR_PARAMS: {
                emu_params = pair.second;
                break;
            }
            case GameField::PATH:
                break;
            default:
                Q_UNREACHABLE();
        }
    }

    game.launch_cmd = QStringLiteral("\"%1\" %2 {file.path}").arg(emu_app, emu_params);
    game.launch_workdir = QFileInfo(emu_app).absolutePath();
}

void store_game(const QFileInfo& finfo, const HashMap<GameField, QString, EnumHash>& fields,
                const Platform& platform, const HashMap<EmulatorId, Emulator>& emulators,
                providers::SearchContext& sctx, std::vector<size_t>& collection_childs)
{
    const QString can_path = finfo.canonicalFilePath();

    if (!sctx.path_to_gameid.count(can_path)) {
        modeldata::Game game(finfo);

        store_game_fields(game, fields, platform, emulators);
        if (game.launch_cmd.isEmpty())
            qWarning().noquote() << MSG_PREFIX << tr_log("game '%1' has no launch command").arg(game.title);

        const size_t game_id = sctx.games.size();
        sctx.path_to_gameid.emplace(can_path, game_id);
        sctx.games.emplace(game_id, std::move(game));
    }

    const size_t game_id = sctx.path_to_gameid.at(can_path);
    collection_childs.emplace_back(game_id);
}

void platform_xml_read_game(QXmlStreamReader& xml, const HashMap<QString, GameField>& field_map,
                            const QString& lb_dir,
                            const Platform& platform, const HashMap<EmulatorId, Emulator>& emulators,
                            providers::SearchContext& sctx, std::vector<size_t>& collection_childs)
{
    HashMap<GameField, QString, EnumHash> game_values;

    while (xml.readNextStartElement()) {
        const auto field_it = field_map.find(xml.name().toString());
        if (field_it == field_map.cend()) {
            xml.skipCurrentElement();
            continue;
        }

        const QString contents = xml.readElementText().trimmed(); // TODO: maybe strrefs
        if (contents.isEmpty())
            continue;

        game_values.emplace(field_it->second, contents);
    }


    // sanity check
    const QString xmlpath = static_cast<QFile*>(xml.device())->fileName();

    const auto path_it = game_values.find(GameField::PATH);
    const bool has_path = path_it != game_values.cend();
    if (!has_path) {
        qWarning().noquote() << MSG_PREFIX << tr_log("a game in `%1` has no path, entry ignored").arg(xmlpath);
        return;
    }

    const QFileInfo game_finfo(lb_dir, path_it->second);
    if (!game_finfo.exists()) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("in `%1`, game file `%2` doesn't seem to exist, entry ignored")
               .arg(xmlpath, path_it->second);
        return;
    }

    store_game(game_finfo, game_values, platform, emulators, sctx, collection_childs);
}

void platform_xml_read_root(QXmlStreamReader& xml, const HashMap<QString, GameField> field_map,
                            const QString& lb_dir,
                            const Platform& platform, const HashMap<EmulatorId, Emulator>& emulators,
                            providers::SearchContext& sctx)
{
    if (!xml.readNextStartElement()) {
        xml.raiseError(tr_log("could not parse `%1`")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }
    if (xml.name() != QLatin1String("LaunchBox")) {
        xml.raiseError(tr_log("`%1` does not have a `<LaunchBox>` root node!")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }


    if (sctx.collections.find(platform.name) == sctx.collections.end())
        sctx.collections.emplace(platform.name, modeldata::Collection(platform.name));

    std::vector<size_t>& collection_childs = sctx.collection_childs[platform.name];


    while (xml.readNextStartElement()) {
        if (xml.name() != QLatin1String("Game")) {
            xml.skipCurrentElement();
            continue;
        }
        platform_xml_read_game(xml, field_map, lb_dir, platform, emulators, sctx, collection_childs);
    }
}

void process_platform_xml(const HashMap<QString, GameField> field_map,
                          const QString& lb_dir,
                          const Platform& platform,
                          const HashMap<EmulatorId, Emulator>& emulators,
                          providers::SearchContext& sctx)
{
    QFile xml_file(platform.xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << MSG_PREFIX << tr_log("could not open `%1`").arg(platform.xml_path);
        return;
    }

    QXmlStreamReader xml(&xml_file);
    platform_xml_read_root(xml, field_map, lb_dir, platform, emulators, sctx);
    if (xml.error())
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
}

struct EmulatorData {
    HashMap<EmulatorId, Emulator> emus;
    std::vector<Platform> platforms;
};
EmulatorData read_emulators_xml(const QString& lb_dir)
{
    const QString xml_path = lb_dir + QLatin1String("Data/Emulators.xml");
    const QString platforms_dir = lb_dir + QLatin1String("Data/Platforms/");

    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << MSG_PREFIX << tr_log("could not open `%1`").arg(xml_path);
        return {};
    }

    EmulatorData out;
    QXmlStreamReader xml(&xml_file);

    if (xml.readNextStartElement() && xml.name() != QLatin1String("LaunchBox")) {
        xml.raiseError(tr_log("`%1` does not have a `<LaunchBox>` root node!")
            .arg(static_cast<QFile*>(xml.device())->fileName()));
    }
    while (xml.readNextStartElement() && !xml.hasError()) {
        if (xml.name() == QLatin1String("EmulatorPlatform")) {
            Platform platform {};
            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("Emulator")) {
                    platform.default_emu_id = xml.readElementText().trimmed();
                    continue;
                }
                if (xml.name() == QLatin1String("Platform")) {
                    platform.name = xml.readElementText().trimmed();
                    continue;
                }
                if (xml.name() == QLatin1String("CommandLine")) {
                    platform.cmd_params = xml.readElementText().trimmed();
                    continue;
                }
                xml.skipCurrentElement();
            }
            if (!platform.name.isEmpty()) {
                platform.xml_path = QFileInfo(platforms_dir % platform.name % QLatin1String(".xml"))
                    .canonicalFilePath();
            }
            if (!platform.incomplete())
                out.platforms.emplace_back(std::move(platform));
            continue;
        }

        if (xml.name() == QLatin1String("Emulator")) {
            EmulatorId emu_id;
            Emulator emu {};
            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("ID")) {
                    emu_id = xml.readElementText().trimmed();
                    continue;
                }
                if (xml.name() == QLatin1String("ApplicationPath")) {
                    const QFileInfo finfo(lb_dir, xml.readElementText().trimmed());
                    emu.app_path = finfo.canonicalFilePath();
                    if (emu.app_path.isEmpty()) {
                        qWarning().noquote() << MSG_PREFIX
                            << tr_log("emulator `%1` doesn't seem to exist, entry ignored")
                               .arg(finfo.filePath());
                    }
                    continue;
                }
                if (xml.name() == QLatin1String("CommandLine")) {
                    emu.cmd_params = xml.readElementText().trimmed();
                    continue;
                }
                xml.skipCurrentElement();
            }
            // assume no collision
            if (!emu_id.isEmpty() && !emu.incomplete())
                out.emus.emplace(std::move(emu_id), std::move(emu));
            continue;
        }

        xml.skipCurrentElement();
    }
    if (xml.error())
        qWarning().noquote() << MSG_PREFIX << xml.errorString();

    // remove platforms without emulator
    for (auto it = out.platforms.begin(); it != out.platforms.end();) {
        const EmulatorId& emu_id = it->default_emu_id;
        if (out.emus.find(emu_id) == out.emus.cend()) {
            qWarning().noquote() << MSG_PREFIX
                << tr_log("emulator platform `%1` refers to an missing emulator id, entry ignored")
                   .arg(it->name);
            it = out.platforms.erase(it);
        }
        else
            ++it;
    }

    return out;
}

QString find_installation()
{
    const QString possible_path = paths::homePath() + QStringLiteral("/LaunchBox/");
    if (QFileInfo::exists(possible_path)) {
        qInfo().noquote() << MSG_PREFIX << tr_log("found directory: `%1`").arg(possible_path);
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

    const EmulatorData emu_data = read_emulators_xml(lb_dir);
    if (emu_data.emus.empty()) {
        qWarning().noquote() << MSG_PREFIX << tr_log("no emulator settings found");
        return;
    }
    if (emu_data.platforms.empty()) {
        qWarning().noquote() << MSG_PREFIX << tr_log("no platforms found");
        return;
    }

    const HashMap<QString, GameField> game_field_map {
        { QStringLiteral("ApplicationPath"), GameField::PATH },
        { QStringLiteral("Title"), GameField::TITLE },
        { QStringLiteral("Developer"), GameField::DEVELOPER },
        { QStringLiteral("Publisher"), GameField::PUBLISHER },
        { QStringLiteral("ReleaseDate"), GameField::RELEASE },
        { QStringLiteral("Notes"), GameField::NOTES },
        { QStringLiteral("PlayMode"), GameField::PLAYMODE },
        { QStringLiteral("Genre"), GameField::GENRE },
        { QStringLiteral("CommunityStarRating"), GameField::STARS },
        { QStringLiteral("Emulator"), GameField::EMULATOR },
        { QStringLiteral("CommandLine"), GameField::EMULATOR_PARAMS },
    };
    const std::vector<std::pair<QString, AssetType>> assetdir_map { // ordered by priority
        { QStringLiteral("Box - Front"), AssetType::BOX_FRONT },
        { QStringLiteral("Box - Front - Reconstructed"), AssetType::BOX_FRONT },
        { QStringLiteral("Fanart - Box - Front"), AssetType::BOX_FRONT },

        { QStringLiteral("Box - Back"), AssetType::BOX_BACK },
        { QStringLiteral("Box - Back - Reconstructed"), AssetType::BOX_BACK },
        { QStringLiteral("Fanart - Box - Back"), AssetType::BOX_BACK },

        { QStringLiteral("Arcade - Marquee"), AssetType::ARCADE_MARQUEE },
        { QStringLiteral("Banner"), AssetType::ARCADE_MARQUEE },

        { QStringLiteral("Cart - Front"), AssetType::CARTRIDGE },
        { QStringLiteral("Disc"), AssetType::CARTRIDGE },
        { QStringLiteral("Fanart - Cart - Front"), AssetType::CARTRIDGE },
        { QStringLiteral("Fanart - Disc"), AssetType::CARTRIDGE },

        { QStringLiteral("Screenshot - Gameplay"), AssetType::SCREENSHOTS },
        { QStringLiteral("Screenshot - Game Select"), AssetType::SCREENSHOTS },
        { QStringLiteral("Screenshot - Game Title"), AssetType::SCREENSHOTS },
        { QStringLiteral("Screenshot - Game Over"), AssetType::SCREENSHOTS },
        { QStringLiteral("Screenshot - High Scores"), AssetType::SCREENSHOTS },

        { QStringLiteral("Advertisement Flyer - Front"), AssetType::POSTER },
        { QStringLiteral("Arcade - Control Panel"), AssetType::ARCADE_PANEL },
        { QStringLiteral("Clear Logo"), AssetType::LOGO },
        { QStringLiteral("Fanart - Background"), AssetType::BACKGROUND },
        { QStringLiteral("Steam Banner"), AssetType::UI_STEAMGRID },
    };
    for (const Platform& platform : emu_data.platforms) {
        process_platform_xml(game_field_map, lb_dir, platform, emu_data.emus, sctx);
        find_assets(lb_dir, platform, assetdir_map, sctx);
    }
}

} // namespace steam
} // namespace providers
