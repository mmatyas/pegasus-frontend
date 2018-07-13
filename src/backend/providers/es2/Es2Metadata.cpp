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
#include "Utils.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QStringBuilder>
#include <QUrl>


namespace {

static constexpr auto MSG_PREFIX = "ES2:";

QString findGamelistFile(const modeldata::Collection& collection)
{
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    const QString GAMELISTFILE = QStringLiteral("/gamelist.xml");

    std::vector<QString> possible_files;
    for (const QString& dir : collection.source_dirs) {
        possible_files.emplace_back(dir % GAMELISTFILE);
    }
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
        // qDebug() << FALLBACK_MSG.arg(path);
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
                                  const HashMap<QString, modeldata::GamePtr>& games_by_shortpath)
{
    // FIXME: except the short path, this function is the same as the Pegasus asset code
    if (!scrapedir.exists())
        return;

    QDirIterator dir_it(scrapedir, QDirIterator::FollowSymlinks);
    while (dir_it.hasNext()) {
        dir_it.next();
        const QFileInfo fileinfo = dir_it.fileInfo();
        const auto detection_result = pegasus_assets::checkFile(fileinfo);
        if (!detection_result.isValid())
            continue;

        const QString shortpath = scrapedir.dirName() % '/' % detection_result.basename;
        if (!games_by_shortpath.count(shortpath))
            continue;

        const modeldata::GamePtr& game = games_by_shortpath.at(shortpath);
        pegasus_assets::addAssetToGame(game->assets, detection_result.asset_type, dir_it.filePath());
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

void findAssets(const modeldata::GamePtr& game_ptr,
                HashMap<MetaTypes, QString, EnumHash>& xml_props,
                const modeldata::Collection& collection)
{
    modeldata::Game& game = *game_ptr;
    const QString rom_dir = collection.source_dirs.constFirst() % '/';

    if (game.assets.single(AssetType::BOX_FRONT).isEmpty()) {
        QString& path = xml_props[MetaTypes::IMAGE];
        resolveShellChars(path, rom_dir);
        if (!path.isEmpty() && ::validExtPath(path))
            pegasus_assets::addAssetToGame(game.assets, AssetType::BOX_FRONT, path);
    }
    if (game.assets.single(AssetType::ARCADE_MARQUEE).isEmpty()) {
        QString& path = xml_props[MetaTypes::MARQUEE];
        resolveShellChars(path, rom_dir);
        if (!path.isEmpty() && ::validExtPath(path))
            pegasus_assets::addAssetToGame(game.assets, AssetType::ARCADE_MARQUEE, path);
    }
    if (xml_props.count(MetaTypes::VIDEO)) {
        QString& path = xml_props[MetaTypes::VIDEO];
        resolveShellChars(path, rom_dir);
        if (!path.isEmpty() && ::validExtPath(path))
            pegasus_assets::addAssetToGame(game.assets, AssetType::VIDEOS, path);
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

void MetadataParser::enhance(const HashMap<QString, modeldata::GamePtr>& games,
                             const HashMap<QString, modeldata::Collection>& collections)
{
    const QString imgdir_base = paths::homePath()
                              % QStringLiteral("/.emulationstation/downloaded_images/");
    // shortpath: dir name + extensionless filename
    HashMap<QString, modeldata::GamePtr> games_by_shortpath;
    games_by_shortpath.reserve(games.size());
    for (const auto& pair : games) {
        const QString shortpath = pair.second->fileinfo().dir().dirName() % '/' % pair.second->fileinfo().completeBaseName();
        games_by_shortpath.emplace(shortpath, pair.second);
    }


    for (const auto& pair : collections) {
        const modeldata::Collection& collection = pair.second;

        // ignore Steam
        if (collection.name() == QLatin1String("Steam"))
            continue;

        // find the metadata file
        const QString gamelist_path = findGamelistFile(collection);
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
        parseGamelistFile(xml, collection, games);
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
                                       const modeldata::Collection& collection,
                                       const HashMap<QString, modeldata::GamePtr>& games) const
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

        parseGameEntry(xml, collection, games);
    }
}

void MetadataParser::parseGameEntry(QXmlStreamReader& xml,
                                    const modeldata::Collection& collection,
                                    const HashMap<QString, modeldata::GamePtr>& games) const
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

    convertToCanonicalPath(game_path, collection.source_dirs.constFirst());
    if (!games.count(game_path))
        return;

    const modeldata::GamePtr& game = games.at(game_path);
    applyMetadata(game, xml_props);
    findAssets(game, xml_props, collection);
}

void MetadataParser::applyMetadata(const modeldata::GamePtr& game_ptr,
                                   HashMap<MetaTypes, QString, EnumHash>& xml_props) const
{
    modeldata::Game& game = *game_ptr;

    // first, the simple strings
    game.title = xml_props[MetaTypes::NAME];
    game.description = xml_props[MetaTypes::DESC];
    game.developers.append(xml_props[MetaTypes::DEVELOPER]);
    game.publishers.append(xml_props[MetaTypes::PUBLISHER]);
    game.genres.append(xml_props[MetaTypes::GENRE]);

    // then the numbers
    game.playcount = xml_props[MetaTypes::PLAYCOUNT].toInt();
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

    game.lastplayed = QDateTime::fromString(xml_props[MetaTypes::LASTPLAYED], m_date_format);

    const QDateTime release_time(QDateTime::fromString(xml_props[MetaTypes::RELEASE], m_date_format));
    game.release_date = release_time.date();
}

} // namespace es2
} // namespace providers
