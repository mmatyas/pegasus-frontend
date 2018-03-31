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

#include "Utils.h"
#include "types/Collection.h"
#include "PegasusAssets.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QStringBuilder>
#include <QUrl>


namespace {

static constexpr auto MSG_PREFIX = "ES2:";

QString findGamelistFile(const Types::Collection& collection)
{
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    const QString GAMELISTFILE = QStringLiteral("/gamelist.xml");

    QVector<QString> possible_files;
    for (const QString& dir : collection.sourceDirs()) {
        possible_files.push_back(dir % GAMELISTFILE);
    }
    if (!collection.shortName().isEmpty()) {
        possible_files.push_back(homePath()
            % QStringLiteral("/.emulationstation/gamelists/")
            % collection.shortName()
            % GAMELISTFILE);
        possible_files.push_back(QStringLiteral("/etc/emulationstation/gamelists/")
            % collection.shortName()
            % GAMELISTFILE);
    }

    for (const auto& path : possible_files) {
        if (validPath(path)) {
            qInfo().noquote() << MSG_PREFIX << QObject::tr("found `%1`").arg(path);
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
        path.replace(0, 1, homePath());
}

void convertToCanonicalPath(QString& path, const QString& containing_dir)
{
    resolveShellChars(path, containing_dir);
    path = QFileInfo(path).canonicalFilePath();
}

void findPegasusAssetsInScrapedir(const QDir& scrapedir, const QHash<QString, Types::Game*>& games_by_shortpath)
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
        if (!games_by_shortpath.contains(shortpath))
            continue;

        Types::Game* const game = games_by_shortpath[shortpath];
        pegasus_assets::addAssetToGame(*game, detection_result.asset_type, dir_it.filePath());
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

inline uint qHash(const MetaTypes& key, uint seed) {
    return ::qHash(static_cast<unsigned char>(key), seed);
}
inline uint qHash(const MetaTypes& key) {
    return ::qHash(static_cast<unsigned char>(key));
}

QHash<QString, MetaTypes>::const_iterator find_by_strref(const QHash<QString, MetaTypes>& map,
                                                         const QStringRef& str)
{
    QHash<QString, MetaTypes>::const_iterator it;
    for (it = map.cbegin(); it != map.cend(); ++it)
        if (it.key() == str)
            break;

    return it;
}

void findAssets(Types::Game& game, QHash<MetaTypes, QString>& xml_props, const Types::Collection& collection)
{
    Types::GameAssets& assets = game.assets();
    const QString rom_dir = collection.sourceDirs().constFirst() % '/';

    if (assets.boxFront().isEmpty()) {
        QString& path = xml_props[MetaTypes::IMAGE];
        resolveShellChars(path, rom_dir);
        if (!path.isEmpty() && validPath(path))
            pegasus_assets::addAssetToGame(game, AssetType::BOX_FRONT, path);
    }
    if (assets.marquee().isEmpty()) {
        QString& path = xml_props[MetaTypes::MARQUEE];
        resolveShellChars(path, rom_dir);
        if (!path.isEmpty() && validPath(path))
            pegasus_assets::addAssetToGame(game, AssetType::ARCADE_MARQUEE, path);
    }
    {
        QString& path = xml_props[MetaTypes::VIDEO];
        resolveShellChars(path, rom_dir);
        if (!path.isEmpty() && validPath(path))
            pegasus_assets::addAssetToGame(game, AssetType::VIDEOS, path);
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

void MetadataParser::enhance(const QHash<QString, Types::Game*>& games,
                             const QHash<QString, Types::Collection*>& collections)
{
    const QString imgdir_base = homePath()
                              % QStringLiteral("/.emulationstation/downloaded_images/");
    // shortpath: dir name + extensionless filename
    QHash<QString, Types::Game*> games_by_shortpath;
    games_by_shortpath.reserve(games.size());
    for (Types::Game* const game : games) {
        const QString shortpath = game->m_fileinfo.dir().dirName() % '/' % game->m_fileinfo.completeBaseName();
        games_by_shortpath.insert(shortpath, game);
    }


    for (const auto& collection_ptr : collections) {
        Types::Collection& collection = *collection_ptr;

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
                                 << QObject::tr("could not open `%1`").arg(gamelist_path);
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
                                       const Types::Collection& collection,
                                       const QHash<QString, Types::Game*>& games) const
{
    // find the root <gameList> element
    if (!xml.readNextStartElement()) {
        xml.raiseError(QObject::tr("could not parse `%1`")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }
    if (xml.name() != QLatin1String("gameList")) {
        xml.raiseError(QObject::tr("`%1` does not have a `<gameList>` root node!")
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
                                    const Types::Collection& collection,
                                    const QHash<QString, Types::Game*>& games) const
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    // read all XML fields into a key-value map
    QHash<MetaTypes, QString> xml_props;
    while (xml.readNextStartElement()) {
        const auto map_iter = find_by_strref(m_key_types, xml.name());
        if (map_iter == m_key_types.cend()) {
            xml.skipCurrentElement();
            continue;
        }

        xml_props[map_iter.value()] = xml.readElementText();
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
            << QObject::tr("the `<game>` node in `%1` that ends at line %2 has no `<path>` parameter")
               .arg(static_cast<QFile*>(xml.device())->fileName())
               .arg(xml.lineNumber());
        return;
    }

    // apply

    convertToCanonicalPath(game_path, collection.sourceDirs().constFirst());
    if (!games.contains(game_path))
        return;

    Types::Game& game = *games[game_path];
    applyMetadata(game, xml_props);
    findAssets(game, xml_props, collection);
}

void MetadataParser::applyMetadata(Types::Game& game,
                                   const QHash<MetaTypes, QString>& xml_props) const
{
    // first, the simple strings
    game.m_title = xml_props[MetaTypes::NAME];
    game.m_description = xml_props[MetaTypes::DESC];
    game.addDeveloper(xml_props[MetaTypes::DEVELOPER]);
    game.addPublisher(xml_props[MetaTypes::PUBLISHER]);
    game.addGenre(xml_props[MetaTypes::GENRE]);

    // then the numbers
    game.m_playcount = xml_props[MetaTypes::PLAYCOUNT].toInt();
    game.m_rating = qBound(0.f, xml_props[MetaTypes::RATING].toFloat(), 1.f);

    // the player count can be a range
    const QString players_field = xml_props[MetaTypes::PLAYERS];
    const auto players_match = m_players_regex.match(players_field);
    if (players_match.hasMatch()) {
        int a = 0, b = 0;
        a = players_match.captured(1).toInt();
        b = players_match.captured(3).toInt();
        game.m_players = std::max(a, b);
    }

    // then the bools
    const QString& favorite_val = xml_props[MetaTypes::FAVORITE];
    if (favorite_val.compare(QLatin1String("yes"), Qt::CaseInsensitive) == 0
        || favorite_val.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
        || favorite_val.compare(QLatin1String("1")) == 0) {
        game.m_favorite = true;
    }

    // then dates
    // NOTE: QDateTime::fromString returns a null (invalid) date on error

    game.m_lastplayed = QDateTime::fromString(xml_props[MetaTypes::LASTPLAYED], m_date_format);

    const QDateTime release_time(QDateTime::fromString(xml_props[MetaTypes::RELEASE], m_date_format));
    game.setRelease(release_time.date());
}

} // namespace es2
} // namespace providers
