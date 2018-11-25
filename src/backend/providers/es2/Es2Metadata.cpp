// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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


#include "Es2Metadata.h"

#include "LocaleUtils.h"
#include "Paths.h"
#include "PegasusAssets.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"
#include "utils/PathCheck.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QStringBuilder>
#include <QUrl>


namespace pegasus_legacy_assets {
struct AssetCheckResult {
    const QString basename;
    const AssetType asset_type;

    bool isValid() const {
        return asset_type != AssetType::UNKNOWN && !basename.isEmpty();
    }
};

AssetCheckResult checkFile(const QFileInfo& file)
{
    const QString basename = file.completeBaseName();
    const int last_dash = basename.lastIndexOf(QChar('-'));
    const QString suffix = (last_dash == -1)
        ? QString()
        : basename.mid(last_dash + 1);

    // missing/unknown suffix -> guess by extension
    const AssetType type = pegasus_assets::str_to_type(suffix);
    if (type == AssetType::UNKNOWN)
        return { basename, pegasus_assets::ext_to_type(file.suffix()) };

    // known suffix but wrong extension -> invalid
    const QString game_basename = basename.left(last_dash);
    if (!pegasus_assets::allowed_asset_exts(type).contains(file.suffix()))
        return { game_basename, AssetType::UNKNOWN };

    // known suffix and valid extension
    return { game_basename, type };
}
} // pegasus_legacy_assets


namespace {

static constexpr auto MSG_PREFIX = "ES2:";

QString findGamelistFile(const modeldata::Collection& collection,
                         const QString& collection_dir)
{
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    const QString GAMELISTFILE = QStringLiteral("/gamelist.xml");

    std::vector<QString> possible_files;
    possible_files.emplace_back(collection_dir % GAMELISTFILE);

    if (!collection.shortName().isEmpty()) {
        possible_files.emplace_back(paths::homePath()
            % QStringLiteral("/.emulationstation/gamelists/")
            % collection.shortName()
            % GAMELISTFILE);
        possible_files.emplace_back(QStringLiteral("/etc/emulationstation/gamelists/")
            % collection.shortName()
            % GAMELISTFILE);
    }

    for (const auto& path : possible_files) {
        if (::validFile(path)) {
            qInfo().noquote() << MSG_PREFIX << tr_log("found `%1`").arg(path);
            return path;
        }
    }

    return QString();
}

void resolveShellChars(QString& path, const QString& containing_dir)
{
    if (path.startsWith(QStringLiteral("./")))
        path.replace(0, 1, containing_dir);
    else if (path.startsWith(QStringLiteral("~/")))
        path.replace(0, 1, paths::homePath());
}

void convertToCanonicalPath(QString& path, const QString& containing_dir)
{
    resolveShellChars(path, containing_dir);
    path = QFileInfo(path).canonicalFilePath();
}

void findPegasusAssetsInScrapedir(const QDir& scrapedir,
                                  const HashMap<QString, modeldata::Game* const>& games_by_shortpath)
{
    // FIXME: except the short path, this function is the same as the Pegasus asset code
    if (!scrapedir.exists())
        return;

    QDirIterator dir_it(scrapedir, QDirIterator::FollowSymlinks);
    while (dir_it.hasNext()) {
        dir_it.next();
        const QFileInfo fileinfo = dir_it.fileInfo();
        const auto detection_result = pegasus_legacy_assets::checkFile(fileinfo);
        if (!detection_result.isValid())
            continue;

        const QString shortpath = scrapedir.dirName() % '/' % detection_result.basename;
        if (!games_by_shortpath.count(shortpath))
            continue;

        modeldata::Game* const game = games_by_shortpath.at(shortpath);
        game->assets.addFileMaybe(detection_result.asset_type, dir_it.filePath());
    }
}

} // namespace


namespace providers {
namespace es2 {

enum class MetaTypes : unsigned char {
    PATH,
    NAME,
    DESC,
    DEVELOPER,
    GENRE,
    PUBLISHER,
    PLAYERS,
    RATING,
    PLAYCOUNT,
    LASTPLAYED,
    RELEASE,
    IMAGE,
    VIDEO,
    MARQUEE,
    FAVORITE,
};

HashMap<QString, MetaTypes>::const_iterator find_by_strref(const HashMap<QString, MetaTypes>& map,
                                                           const QStringRef& str)
{
    HashMap<QString, MetaTypes>::const_iterator it;
    for (it = map.cbegin(); it != map.cend(); ++it)
        if (it->first == str)
            break;

    return it;
}

void findAssets(modeldata::Game& game,
                HashMap<MetaTypes, QString, EnumHash>& xml_props,
                const QString& collection_dir)
{
    const QString rom_dir = collection_dir % '/';

    if (game.assets.single(AssetType::BOX_FRONT).isEmpty()) {
        QString& path = xml_props[MetaTypes::IMAGE];
        resolveShellChars(path, rom_dir);
        if (!path.isEmpty() && ::validExtPath(path))
            game.assets.addFileMaybe(AssetType::BOX_FRONT, path);
    }
    if (game.assets.single(AssetType::ARCADE_MARQUEE).isEmpty()) {
        QString& path = xml_props[MetaTypes::MARQUEE];
        resolveShellChars(path, rom_dir);
        if (!path.isEmpty() && ::validExtPath(path))
            game.assets.addFileMaybe(AssetType::ARCADE_MARQUEE, path);
    }
    if (xml_props.count(MetaTypes::VIDEO)) {
        QString& path = xml_props[MetaTypes::VIDEO];
        resolveShellChars(path, rom_dir);
        if (!path.isEmpty() && ::validExtPath(path))
            game.assets.addFileMaybe(AssetType::VIDEOS, path);
    }
}

MetadataParser::MetadataParser(QObject* parent)
    : QObject(parent)
    , m_key_types {
        { QStringLiteral("path"), MetaTypes::PATH },
        { QStringLiteral("name"), MetaTypes::NAME },
        { QStringLiteral("desc"), MetaTypes::DESC },
        { QStringLiteral("developer"), MetaTypes::DEVELOPER },
        { QStringLiteral("genre"), MetaTypes::GENRE },
        { QStringLiteral("publisher"), MetaTypes::PUBLISHER },
        { QStringLiteral("players"), MetaTypes::PLAYERS },
        { QStringLiteral("rating"), MetaTypes::RATING },
        { QStringLiteral("playcount"), MetaTypes::PLAYCOUNT },
        { QStringLiteral("lastplayed"), MetaTypes::LASTPLAYED },
        { QStringLiteral("releasedate"), MetaTypes::RELEASE },
        { QStringLiteral("image"), MetaTypes::IMAGE },
        { QStringLiteral("video"), MetaTypes::VIDEO },
        { QStringLiteral("marquee"), MetaTypes::MARQUEE },
        { QStringLiteral("favorite"), MetaTypes::FAVORITE },
    }
    , m_date_format(QStringLiteral("yyyyMMdd'T'HHmmss"))
    , m_players_regex(QStringLiteral("(\\d+)(-(\\d+))?"))
{}

void MetadataParser::enhance(HashMap<QString, modeldata::Game>& games,
                             const HashMap<QString, modeldata::Collection>& collections,
                             const HashMap<QString, std::vector<QString>>& collection_childs,
                             const HashMap<QString, QString>& collection_dirs)
{
    const QString imgdir_base = paths::homePath()
                              % QStringLiteral("/.emulationstation/downloaded_images/");
    // shortpath: dir name + extensionless filename
    HashMap<QString, modeldata::Game* const> games_by_shortpath;
    games_by_shortpath.reserve(games.size());
    for (const auto& coll_titles_pair : collection_childs) {
        Q_ASSERT(collections.count(coll_titles_pair.first));
        const QString coll_shortname = collections.at(coll_titles_pair.first).shortName();

        for (const auto& title : coll_titles_pair.second) {
            Q_ASSERT(games.count(title));
            modeldata::Game* const game = &games.at(title);

            const QString gamefile = game->fileinfo().completeBaseName();
            const QString shortpath = coll_shortname % '/' % gamefile;
            games_by_shortpath.emplace(shortpath, game);
        }
    }


    for (const auto& pair : collections) {
        const modeldata::Collection& collection = pair.second;

        // ignore Steam
        if (collection.name() == QLatin1String("Steam"))
            continue;

        // find the metadata file
        const QString collection_dir = collection_dirs.at(collection.name());
        const QString gamelist_path = findGamelistFile(collection, collection_dir);
        if (gamelist_path.isEmpty())
            continue;

        // open the file
        QFile xml_file(gamelist_path);
        if (!xml_file.open(QIODevice::ReadOnly)) {
            qWarning().noquote() << MSG_PREFIX
                                 << tr_log("could not open `%1`").arg(gamelist_path);
            continue;
        }

        // parse the file
        QXmlStreamReader xml(&xml_file);
        parseGamelistFile(xml, games, collection_dir);
        if (xml.error())
            qWarning().noquote() << MSG_PREFIX << xml.errorString();

        // search for assets in `downloaded_images`
        if (!collection.shortName().isEmpty()) {
            constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
            const QDir imgdir(imgdir_base % collection.shortName(), QString(), QDir::NoSort, dir_filters);
            findPegasusAssetsInScrapedir(imgdir, games_by_shortpath);
        }
    }
}

void MetadataParser::parseGamelistFile(QXmlStreamReader& xml,
                                       HashMap<QString, modeldata::Game>& games,
                                       const QString& collection_dir) const
{
    // find the root <gameList> element
    if (!xml.readNextStartElement()) {
        xml.raiseError(tr_log("could not parse `%1`")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }
    if (xml.name() != QLatin1String("gameList")) {
        xml.raiseError(tr_log("`%1` does not have a `<gameList>` root node!")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }

    // read all <game> nodes
    while (xml.readNextStartElement()) {
        if (xml.name() != QLatin1String("game")) {
            xml.skipCurrentElement();
            continue;
        }

        parseGameEntry(xml, games, collection_dir);
    }
}

void MetadataParser::parseGameEntry(QXmlStreamReader& xml,
                                    HashMap<QString, modeldata::Game>& games,
                                    const QString& collection_dir) const
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    // read all XML fields into a key-value map
    HashMap<MetaTypes, QString, EnumHash> xml_props;
    while (xml.readNextStartElement()) {
        const auto map_iter = find_by_strref(m_key_types, xml.name());
        if (map_iter == m_key_types.cend()) {
            xml.skipCurrentElement();
            continue;
        }

        xml_props[map_iter->second] = xml.readElementText();
    }
    if (xml.error()) {
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
        return;
    }

    // check if all required params are present
    QString& game_path = xml_props[MetaTypes::PATH];
    if (game_path.isEmpty()) {
        qWarning().noquote()
            << MSG_PREFIX
            << tr_log("the `<game>` node in `%1` that ends at line %2 has no `<path>` parameter")
               .arg(static_cast<QFile*>(xml.device())->fileName())
               .arg(xml.lineNumber());
        return;
    }

    // apply

    convertToCanonicalPath(game_path, collection_dir);
    if (!games.count(game_path))
        return;

    modeldata::Game& game = games.at(game_path);
    applyMetadata(game, xml_props);
    findAssets(game, xml_props, collection_dir);
}

void MetadataParser::applyMetadata(modeldata::Game& game,
                                   HashMap<MetaTypes, QString, EnumHash>& xml_props) const
{
    // first, the simple strings
    game.title = xml_props[MetaTypes::NAME];
    game.description = xml_props[MetaTypes::DESC];
    game.developers.append(xml_props[MetaTypes::DEVELOPER]);
    game.publishers.append(xml_props[MetaTypes::PUBLISHER]);
    game.genres.append(xml_props[MetaTypes::GENRE]);

    // then the numbers
    game.playcount += xml_props[MetaTypes::PLAYCOUNT].toInt();
    game.rating = qBound(0.f, xml_props[MetaTypes::RATING].toFloat(), 1.f);

    // the player count can be a range
    const QString players_field = xml_props[MetaTypes::PLAYERS];
    const auto players_match = m_players_regex.match(players_field);
    if (players_match.hasMatch()) {
        int a = 0, b = 0;
        a = players_match.captured(1).toInt();
        b = players_match.captured(3).toInt();
        game.player_count = std::max(a, b);
    }

    // then the bools
    const QString& favorite_val = xml_props[MetaTypes::FAVORITE];
    if (favorite_val.compare(QLatin1String("yes"), Qt::CaseInsensitive) == 0
        || favorite_val.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
        || favorite_val.compare(QLatin1String("1")) == 0) {
        game.is_favorite |= true;
    }

    // then dates
    // NOTE: QDateTime::fromString returns a null (invalid) date on error

    game.last_played = QDateTime::fromString(xml_props[MetaTypes::LASTPLAYED], m_date_format);

    const QDateTime release_time(QDateTime::fromString(xml_props[MetaTypes::RELEASE], m_date_format));
    game.release_date = release_time.date();
}

} // namespace es2
} // namespace providers
