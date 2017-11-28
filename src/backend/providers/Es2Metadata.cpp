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

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QUrl>
#include <QXmlStreamReader>


namespace {

static constexpr auto MSG_PREFIX = "ES2:";
static constexpr auto GAMELISTFILE = "/gamelist.xml";

static const QString KEY_PATH(QStringLiteral("path"));
static const QString KEY_NAME(QStringLiteral("name"));
static const QString KEY_DESC(QStringLiteral("desc"));
static const QString KEY_DEVELOPER(QStringLiteral("developer"));
static const QString KEY_GENRE(QStringLiteral("genre"));
static const QString KEY_PUBLISHER(QStringLiteral("publisher"));
static const QString KEY_PLAYERS(QStringLiteral("players"));
static const QString KEY_RATING(QStringLiteral("rating"));
static const QString KEY_PLAYCOUNT(QStringLiteral("playcount"));
static const QString KEY_LASTPLAYED(QStringLiteral("lastplayed"));
static const QString KEY_RELEASE(QStringLiteral("releasedate"));
static const QString KEY_IMAGE(QStringLiteral("image"));
static const QString KEY_VIDEO(QStringLiteral("video"));
static const QString KEY_MARQUEE(QStringLiteral("marquee"));
static const QString KEY_FAVORITE(QStringLiteral("favorite"));

static const QHash<QString, QString> EMPTY_GAMEENTRY = {
    { KEY_PATH, QString() },
    { KEY_NAME, QString() },
    { KEY_DESC, QString() },
    { KEY_DEVELOPER, QString() },
    { KEY_GENRE, QString() },
    { KEY_PUBLISHER, QString() },
    { KEY_PLAYERS, QString() },
    { KEY_RATING, QString() },
    { KEY_PLAYCOUNT, QString() },
    { KEY_LASTPLAYED, QString() },
    { KEY_RELEASE, QString() },
    { KEY_IMAGE, QString() },
    { KEY_VIDEO, QString() },
    { KEY_MARQUEE, QString() },
    { KEY_FAVORITE, QString() },
};

static const QString DATEFORMAT(QStringLiteral("yyyyMMdd'T'HHmmss"));
static const QRegularExpression PLAYERSREGEX(QStringLiteral("(\\d+)(-(\\d+))?"));


void parseGameEntry(QXmlStreamReader&, const Types::Collection&, const QHash<QString, Types::Game*>&);
void applyMetadata(Types::Game&, const QHash<QString, QString>&);
void findAssets(Types::Game&, QHash<QString, QString>&, const Types::Collection&);
QHash<QString, QString>::iterator findByStrRef(QHash<QString, QString>&, const QStringRef&);
void convertToCanonicalPath(QString& path, const QString& containing_dir);


QString findGamelistFile(const Types::Collection& collection)
{
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    const QVector<QString> possible_files = {
        collection.sourceDirs().constFirst() % GAMELISTFILE,
        homePath() % QStringLiteral("/.emulationstation/gamelists/") % collection.tag() % GAMELISTFILE,
        QStringLiteral("/etc/emulationstation/gamelists/") % collection.tag() % GAMELISTFILE,
    };

    for (const auto& path : possible_files) {
        if (validPath(path)) {
            qInfo().noquote() << MSG_PREFIX << QObject::tr("found `%1`").arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

void parseGamelistFile(QXmlStreamReader& xml,
                       const Types::Collection& collection,
                       const QHash<QString, Types::Game*>& games)
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

void parseGameEntry(QXmlStreamReader& xml,
                    const Types::Collection& collection,
                    const QHash<QString, Types::Game*>& games)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    // read all XML fields into a key-value map
    QHash<QString, QString> xml_props = EMPTY_GAMEENTRY;
    while (xml.readNextStartElement()) {
        auto it = findByStrRef(xml_props, xml.name());
        if (it != xml_props.end())
            it.value() = xml.readElementText();
        else
            xml.skipCurrentElement();
    }
    if (xml.error()) {
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
        return;
    }

    // check if all required params are present
    QString& game_path = xml_props[KEY_PATH];
    if (game_path.isEmpty()) {
        qWarning().noquote()
            << MSG_PREFIX
            << QObject::tr("the `<game>` node in `%1` that ends at line %2 has no `<%3>` parameter")
               .arg(static_cast<QFile*>(xml.device())->fileName())
               .arg(xml.lineNumber())
               .arg(KEY_PATH);
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

void applyMetadata(Types::Game& game, const QHash<QString, QString>& xml_props)
{
    // first, the simple strings
    game.m_title = xml_props[KEY_NAME];
    game.m_description = xml_props[KEY_DESC];
    game.m_developer = xml_props[KEY_DEVELOPER];
    game.m_genre = xml_props[KEY_GENRE];
    game.m_publisher = xml_props[KEY_PUBLISHER];

    // then the numbers
    game.m_playcount = xml_props[KEY_PLAYCOUNT].toInt();
    game.m_rating = qBound(0.f, xml_props[KEY_PLAYCOUNT].toFloat(), 1.f);

    // the player count can be a range
    const QString players_field = xml_props[KEY_PLAYERS];
    const auto players_match = PLAYERSREGEX.match(players_field);
    if (players_match.hasMatch()) {
        int a = 0, b = 0;
        a = players_match.captured(1).toInt();
        b = players_match.captured(3).toInt();
        game.m_players = std::max(a, b);
    }

    // then the bools
    const QString& favorite_val = xml_props[KEY_FAVORITE];
    if (favorite_val.compare(QLatin1String("yes"), Qt::CaseInsensitive) == 0
        || favorite_val.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
        || favorite_val.compare(QLatin1String("1")) == 0) {
        game.m_favorite = true;
    }

    // then dates
    // NOTE: QDateTime::fromString returns a null (invalid) date on error

    game.m_lastplayed = QDateTime::fromString(xml_props[KEY_LASTPLAYED], DATEFORMAT);

    const QDateTime release_date(QDateTime::fromString(xml_props[KEY_RELEASE], DATEFORMAT));
    if (release_date.isValid()) {
        const QDate date(release_date.date());
        game.m_year = date.year();
        game.m_month = date.month();
        game.m_day = date.day();
    }
}

void findAssets(Types::Game& game, QHash<QString, QString>& xml_props, const Types::Collection& collection)
{
    Types::GameAssets& assets = game.assets();
    const QString rom_dir = collection.sourceDirs().constFirst() % '/';

    if (assets.boxFront().isEmpty()) {
        QString& path = xml_props[KEY_IMAGE];
        convertToCanonicalPath(path, rom_dir);
        if (!path.isEmpty() && validPath(path))
            assets.setSingle(AssetType::BOX_FRONT, QUrl::fromLocalFile(path).toString());
    }
    if (assets.marquee().isEmpty()) {
        QString& path = xml_props[KEY_MARQUEE];
        convertToCanonicalPath(path, rom_dir);
        if (!path.isEmpty() && validPath(path))
            assets.setSingle(AssetType::MARQUEE, QUrl::fromLocalFile(path).toString());
    }
    {
        QString& path = xml_props[KEY_VIDEO];
        convertToCanonicalPath(path, rom_dir);
        if (!path.isEmpty() && validPath(path))
            assets.appendMulti(AssetType::VIDEOS, QUrl::fromLocalFile(path).toString());
    }

    // search for assets in ~/.emulationstation/downloaded_images

    const QString path_base = homePath()
                              % QStringLiteral("/.emulationstation/downloaded_images/")
                              % collection.tag() % '/'
                              % game.m_rom_basename;

    for (auto asset_type : Assets::singleTypes) {
        if (assets.m_single_assets[asset_type].isEmpty()) {
            assets.m_single_assets[asset_type] = Assets::findFirst(asset_type, path_base);
        }
    }
    for (auto asset_type : Assets::multiTypes) {
        assets.m_multi_assets[asset_type].append(Assets::findAll(asset_type, path_base));
    }
}

QHash<QString, QString>::iterator findByStrRef(QHash<QString, QString>& map, const QStringRef& str)
{
    QHash<QString, QString>::iterator it;
    for (it = map.begin(); it != map.end(); ++it)
        if (it.key() == str)
            break;

    return it;
}

void convertToCanonicalPath(QString& path, const QString& containing_dir)
{
    static const QString HOMESLASH(QStringLiteral("~/"));
    static const QString DOTSLASH(QStringLiteral("./"));

    if (path.startsWith(DOTSLASH))
        path.replace(0, 1, containing_dir);
    else if (path.startsWith(HOMESLASH))
        path.replace(0, 1, homePath());

    path = QFileInfo(path).canonicalFilePath();
}

} // namespace


namespace providers {

void Es2Metadata::fill(const QHash<QString, Types::Game*>& games,
                       const QHash<QString, Types::Collection*>& collections,
                       const QVector<QString>&)
{
    for (const auto& collection_ptr : collections) {
        Types::Collection& collection = *collection_ptr;

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
    }
}

} // namespace providers
