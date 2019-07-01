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
};

// FIXME: Very slow!
HashMap<size_t, modeldata::Game>::iterator
find_game_by_title(const QString& title,
                   const std::vector<size_t>& coll_childs, HashMap<size_t, modeldata::Game>& games)
{
    const auto child_it = std::find_if(coll_childs.cbegin(), coll_childs.cend(),
        [&title, &games](const size_t gameid){
            Q_ASSERT(games.find(gameid) != games.cend());
            return games.at(gameid).title == title;
        });

    if (child_it != coll_childs.cend())
        return games.find(*child_it);

    return games.end();
}

void find_assets(const QString& lb_dir, const QString& collection_name,
                 const std::vector<std::pair<QString, AssetType>>& assetdir_map,
                 providers::SearchContext& sctx)
{
    const auto coll_childs_it = sctx.collection_childs.find(collection_name);
    if (coll_childs_it == sctx.collection_childs.cend())
        return;

    std::vector<size_t>& collection_childs = coll_childs_it->second;


    const QString images_root = lb_dir % QLatin1String("Images/") % collection_name % QChar('/');

    // constexpr auto dirs_only = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto files_only = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto recursive = QDirIterator::Subdirectories;

    for (const auto& assetdir_pair : assetdir_map) {
        const QString assetdir_path = images_root + assetdir_pair.first;
        const AssetType assetdir_type = assetdir_pair.second;

        std::vector<QString> paths; // for manual sorting
        QDirIterator file_it(assetdir_path, files_only, recursive);
        while (file_it.hasNext())
            paths.emplace_back(file_it.next());

        VEC_SORT(paths);
        for (const QString& path : paths) {
            const QString basename = QFileInfo(path).completeBaseName();
            const QString game_title = basename.left(basename.length() - 3); // gamename "-xx" .ext

            const auto it = find_game_by_title(game_title, collection_childs, sctx.games);
            if (it == sctx.games.cend())
                continue;

            modeldata::Game& game = it->second;
            game.assets.addFileMaybe(assetdir_type, path);
        }
    }
}

void store_game_fields(modeldata::Game& game, const HashMap<GameField, QString>& fields)
{
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
            case GameField::PATH:
                break;
            default:
                Q_UNREACHABLE();
        }
    }
}

void store_game(const QFileInfo& finfo, const HashMap<GameField, QString>& fields,
                providers::SearchContext& sctx, std::vector<size_t>& collection_childs)
{
    const QString can_path = finfo.canonicalFilePath();

    if (!sctx.path_to_gameid.count(can_path)) {
        modeldata::Game game(finfo);

        store_game_fields(game, fields);
        // FIXME: launch cmd

        const size_t game_id = sctx.games.size();
        sctx.path_to_gameid.emplace(can_path, game_id);
        sctx.games.emplace(game_id, std::move(game));
    }

    const size_t game_id = sctx.path_to_gameid.at(can_path);
    collection_childs.emplace_back(game_id);
}

void xml_read_game(const QString& lb_dir, QXmlStreamReader& xml, const HashMap<QString, GameField> field_map,
                   providers::SearchContext& sctx, std::vector<size_t>& collection_childs)
{
    HashMap<GameField, QString> game_values;

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

    const QString game_path = lb_dir + path_it->second;
    const QFileInfo game_finfo(game_path);
    if (!game_finfo.exists()) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("in `%1`, game file `%2` doesn't seem to exist, entry ignored")
               .arg(xmlpath, game_path);
        return;
    }


    store_game(game_finfo, game_values, sctx, collection_childs);
}

void xml_read_root(const QString& lb_dir, QXmlStreamReader& xml, const HashMap<QString, GameField> field_map,
                   const QString& collection_name, providers::SearchContext& sctx)
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


    // create or get collection
    auto collection_it = sctx.collections.find(collection_name);
    if (collection_it == sctx.collections.end())
        collection_it = sctx.collections.emplace(collection_name, modeldata::Collection(collection_name)).first;

    std::vector<size_t>& collection_childs = sctx.collection_childs[collection_name];
    // FIXME: launch command
    // modeldata::Collection& collection = collection_it->second;


    while (xml.readNextStartElement()) {
        if (xml.name() != QLatin1String("Game")) {
            xml.skipCurrentElement();
            continue;
        }

        xml_read_game(lb_dir, xml, field_map, sctx, collection_childs);
    }
}

void process_xml(const QString& lb_dir, const QString& xml_path, const HashMap<QString, GameField> field_map,
                 const QString collection_name, providers::SearchContext& sctx)
{
    Q_ASSERT(!lb_dir.isEmpty());
    Q_ASSERT(!xml_path.isEmpty());

    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << MSG_PREFIX << tr_log("could not open `%1`").arg(xml_path);
        return;
    }

    QXmlStreamReader xml(&xml_file);
    xml_read_root(lb_dir, xml, field_map, collection_name, sctx);
    if (xml.error())
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
}

std::vector<QString> find_xmls(const QString& lb_dir)
{
    Q_ASSERT(!lb_dir.isEmpty());

    const QString xml_dir = lb_dir + QLatin1String("Data/Platforms/");
    const QStringList name_filters { QStringLiteral("*.xml") };
    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;

    const QFileInfoList finfos = QDir(xml_dir).entryInfoList(name_filters, dir_filters);
    if (finfos.isEmpty()) {
        qInfo().noquote() << MSG_PREFIX << tr_log("no platforms seem to be set up yet");
        return {};
    }

    std::vector<QString> out;
    out.reserve(static_cast<size_t>(finfos.size()));
    for (const QFileInfo& finfo : finfos)
        out.emplace_back(finfo.canonicalFilePath());

    return out;
}

QString find_installation()
{
    // TODO: This should be configurable on the UI side
    const QString possible_path = paths::homePath() + QStringLiteral("/LaunchBox/");
    if (QFileInfo::exists(possible_path)) {
        qInfo().noquote() << MSG_PREFIX << tr_log("found directory: `%1`").arg(possible_path);
        return possible_path;
    }

    qInfo().noquote() << MSG_PREFIX << tr_log("no installation found");
    return {};
}
} // namespace


namespace providers {
namespace launchbox {

LaunchboxProvider::LaunchboxProvider(QObject* parent)
    : Provider(QStringLiteral("LaunchBox"), PROVIDES_GAMES, parent)
{}

void LaunchboxProvider::findLists(providers::SearchContext& sctx)
{
    const QString lb_dir = find_installation();
    if (lb_dir.isEmpty())
        return;

    const std::vector<QString> xml_paths = find_xmls(lb_dir);
    if (xml_paths.empty())
        return;

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

    for (const QString& path : xml_paths) {
        const QString collection_name = QFileInfo(path).baseName();
        process_xml(lb_dir, path, game_field_map, collection_name, sctx);
        find_assets(lb_dir, collection_name, assetdir_map, sctx);
    }
}

} // namespace steam
} // namespace providers
