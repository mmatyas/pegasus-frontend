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

#include "Model.h"
#include "Utils.h"

#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QXmlStreamReader>

static constexpr auto MSG_PREFIX = "ES2: ";


namespace model_providers {

Es2Metadata::Es2Metadata()
{
}

void Es2Metadata::fill(const Model::Platform& platform)
{
    // find the metadata file
    const QString xml_path = findGamelistFile(platform);
    if (xml_path.isEmpty()) {
        qWarning().noquote()
            << MSG_PREFIX
            << QObject::tr("gamelist for platform `%1` not found").arg(platform.m_short_name);
        return;
    }

    // open the file
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << MSG_PREFIX << QObject::tr("Could not open `%1`").arg(xml_path);
        return;
    }

    // parse the file
    QXmlStreamReader xml(&xml_file);
    parseGamelistFile(xml, platform);
    if (xml.error())
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
}

QString Es2Metadata::findGamelistFile(const Model::Platform& platform)
{
    Q_ASSERT(!platform.m_short_name.isEmpty());

    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";
    static constexpr auto FILENAME = "/gamelist.xml";

    const QVector<QString> possible_dirs = {
        platform.m_rom_dir_path,
        homePath() % "/.emulationstation/gamelists/" % platform.m_short_name,
        "/etc/emulationstation/gamelists/" % platform.m_short_name,
    };

    for (const auto& dir : possible_dirs) {
        const QString path = dir % FILENAME;
        if (validPath(path)) {
            qInfo().noquote() << MSG_PREFIX << QObject::tr("found `%1`").arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

void Es2Metadata::parseGamelistFile(QXmlStreamReader& xml, const Model::Platform& platform)
{
    Q_ASSERT(!platform.games().isEmpty());

    // Build a path -> game map for quick access.
    // To find matches between the real files and the ones in the gamelist,
    // their canonical path will be compared.
    QHash<QString, Model::Game*> game_by_path;
    for (Model::Game* game : qAsConst(platform.games()))
        game_by_path.insert(QFileInfo(game->m_rom_path).canonicalFilePath(), game);


    // find the root <gameList> element
    if (!xml.readNextStartElement()) {
        xml.raiseError(QObject::tr("could not parse `%1`")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }
    if (xml.name() != "gameList") {
        xml.raiseError(QObject::tr("`%1` does not have a `<gameList>` root node!")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }

    // read all <game> nodes
    while (xml.readNextStartElement()) {
        if (xml.name() != "game") {
            xml.skipCurrentElement();
            continue;
        }

        parseGameEntry(xml, platform, game_by_path);
    }
}

void Es2Metadata::parseGameEntry(QXmlStreamReader& xml,
                                 const Model::Platform& platform,
                                 QHash<QString, Model::Game*>& game_by_path)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    static const QString PATH_TAG = "path";

    // read all XML fields into a key-value map
    QHash<QString, QString> xml_props;
    while (xml.readNextStartElement())
        xml_props.insert(xml.name().toString(), xml.readElementText());

    // check if all required params are present
    if (!xml_props.contains(PATH_TAG)) {
        qWarning().noquote()
            << MSG_PREFIX
            << QObject::tr("the `<game>` node in `%1` that ends at line #%2 has no `<%3>` parameter")
               .arg(static_cast<QFile*>(xml.device())->fileName())
               .arg(xml.lineNumber())
               .arg(PATH_TAG);
        return;
    }

    // find the matching game
    // NOTE: every game (path) appears only once, so we can take() it out of the map
    const QString path = platform.m_rom_dir_path % "/" % xml_props[PATH_TAG];
    const QString canonical_path = QFileInfo(path).canonicalFilePath();
    Model::Game* game = game_by_path.take(canonical_path);
    if (!game)
        return;

    applyMetadata(*game, platform, xml_props);
}

void Es2Metadata::applyMetadata(Model::Game& game, const Model::Platform& platform,
                                const QHash<QString, QString>& xml_props)
{
    // this function will run quite often; let's cache some variables
    static const QString KEY_NAME("name");
    static const QString KEY_DESC("desc");
    static const QString KEY_DEVELOPER("developer");
    static const QString KEY_GENRE("genre");
    static const QString KEY_PUBLISHER("publisher");
    static const QString KEY_PLAYERS("players");
    static const QString KEY_RATING("rating");
    static const QString KEY_PLAYCOUNT("playcount");
    static const QString KEY_LASTPLAYED("lastplayed");
    static const QString KEY_RELEASE("releasedate");
    static const QString KEY_IMAGE("image");
    static const QString KEY_VIDEO("video");
    static const QString KEY_MARQUEE("marquee");
    static const QString KEY_FAVORITE("favorite");

    static const QString DATEFORMAT("yyyyMMdd'T'HHmmss");

    // apply the previously read values

    // first, the simple strings
    game.m_title = xml_props.value(KEY_NAME);
    game.m_description = xml_props.value(KEY_DESC);
    game.m_developer = xml_props.value(KEY_DEVELOPER);
    game.m_genre = xml_props.value(KEY_GENRE);
    game.m_publisher = xml_props.value(KEY_PUBLISHER);

    // then the numbers
    parseStoreInt(xml_props.value(KEY_PLAYERS), game.m_players);
    parseStoreInt(xml_props.value(KEY_PLAYCOUNT), game.m_playcount);
    parseStoreFloat(xml_props.value(KEY_RATING), game.m_rating);

    game.m_rating = qBound(0.f, game.m_rating, 1.f);

    // then the bools
    const QString& favorite_val = xml_props.value(KEY_FAVORITE);
    if (favorite_val.compare(QLatin1String("yes"), Qt::CaseInsensitive) == 0
        || favorite_val.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
        || favorite_val.compare(QLatin1String("1")) == 0) {
        game.m_favorite = true;
    }

    // then dates

    // NOTE: QDateTime::fromString returns a null (invalid) date on error
    game.m_lastplayed = QDateTime::fromString(xml_props.value(KEY_LASTPLAYED), DATEFORMAT);

    const QDateTime release_date(QDateTime::fromString(xml_props.value(KEY_RELEASE), DATEFORMAT));
    if (release_date.isValid()) {
        const QDate date(release_date.date());
        game.m_year = date.year();
        game.m_month = date.month();
        game.m_day = date.day();
    }

    // and also see if we can set the assets

    Model::GameAssets& assets = *game.assets();

    const QString rom_dir_prefix = platform.m_rom_dir_path % '/';

    if (assets.boxFront().isEmpty()) {
        QString path = xml_props.value(KEY_IMAGE);
        convertToAbsolutePath(path, rom_dir_prefix);
        if (!path.isEmpty() && validPath(path))
            assets.setSingle(Assets::Type::BOX_FRONT, path);
    }
    if (assets.marquee().isEmpty()) {
        QString path = xml_props.value(KEY_MARQUEE);
        convertToAbsolutePath(path, rom_dir_prefix);
        if (!path.isEmpty() && validPath(path))
            assets.setSingle(Assets::Type::MARQUEE, path);
    }
    {
        QString path = xml_props.value(KEY_VIDEO);
        convertToAbsolutePath(path, rom_dir_prefix);
        if (!path.isEmpty() && validPath(path))
            assets.appendMulti(Assets::Type::VIDEOS, path);
    }

    // search for assets in ~/.emulationstation/downloaded_images

    const QString path_base = homePath()
                              % QStringLiteral("/.emulationstation/downloaded_images/")
                              % platform.m_short_name % '/'
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

void Es2Metadata::convertToAbsolutePath(QString& path, const QString& root_dir_prefix)
{
    static const QRegularExpression HOMESTART("^~");

    path.replace(HOMESTART, homePath());
    if (path.startsWith('.'))
        path.prepend(root_dir_prefix);
}

} // namespace model_providers
