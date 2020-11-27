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


#include "Es2Metadata.h"

#include "LocaleUtils.h"
#include "Log.h"
#include "Paths.h"
#include "model/gaming/Assets.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "providers/SearchContext.h"
#include "providers/es2/Es2Systems.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QStringBuilder>
#include <QXmlStreamReader>


namespace {

QString find_gamelist_xml(const std::vector<QString>& possible_config_dirs, const QDir& system_dir, const QString& system_name)
{
    const QString GAMELISTFILE = QStringLiteral("/gamelist.xml");

    std::vector<QString> possible_files { system_dir.path() % GAMELISTFILE };

    if (!system_name.isEmpty()) {
        for (const QString& dir_path : possible_config_dirs) {
            possible_files.emplace_back(dir_path
                % QStringLiteral("/gamelists/")
                % system_name
                % GAMELISTFILE);
        }
    }

    for (const auto& path : possible_files) {
        if (QFileInfo::exists(path))
            return path;
    }

    return {};
}

QString shell_to_canonical_path(const QDir& base_dir, const QString& shell_filepath)
{
    if (shell_filepath.isEmpty())
        return {};

    const QString real_path = shell_filepath.startsWith(QLatin1String("~/"))
        ? paths::homePath() + shell_filepath.midRef(1)
        : shell_filepath;
    return QFileInfo(base_dir, real_path).canonicalFilePath();
}

} // namespace


namespace providers {
namespace es2 {

enum class MetaType : unsigned char {
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

Metadata::Metadata(QString log_tag, std::vector<QString> possible_config_dirs)
    : m_log_tag(std::move(log_tag))
    , m_config_dirs(std::move(possible_config_dirs))
    , m_key_types {
        { QStringLiteral("path"), MetaType::PATH },
        { QStringLiteral("name"), MetaType::NAME },
        { QStringLiteral("desc"), MetaType::DESC },
        { QStringLiteral("developer"), MetaType::DEVELOPER },
        { QStringLiteral("genre"), MetaType::GENRE },
        { QStringLiteral("publisher"), MetaType::PUBLISHER },
        { QStringLiteral("players"), MetaType::PLAYERS },
        { QStringLiteral("rating"), MetaType::RATING },
        { QStringLiteral("playcount"), MetaType::PLAYCOUNT },
        { QStringLiteral("lastplayed"), MetaType::LASTPLAYED },
        { QStringLiteral("releasedate"), MetaType::RELEASE },
        { QStringLiteral("image"), MetaType::IMAGE },
        { QStringLiteral("video"), MetaType::VIDEO },
        { QStringLiteral("marquee"), MetaType::MARQUEE },
        { QStringLiteral("favorite"), MetaType::FAVORITE },
    }
    , m_date_format(QStringLiteral("yyyyMMdd'T'HHmmss"))
    , m_players_regex(QStringLiteral("(\\d+)(-(\\d+))?"))
    , m_asset_type_map {  // TODO: C++14 with constexpr pair ctor
        { MetaType::IMAGE, AssetType::BOX_FRONT },
        { MetaType::MARQUEE, AssetType::ARCADE_MARQUEE },
        { MetaType::VIDEO, AssetType::VIDEO },
    }
{}

HashMap<MetaType, QString, EnumHash> Metadata::parse_gamelist_game_node(QXmlStreamReader& xml) const
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    HashMap<MetaType, QString, EnumHash> xml_props;
    while (xml.readNextStartElement()) {
        const auto it = std::find_if(
            m_key_types.cbegin(),
            m_key_types.cend(),
            [&xml](const decltype(m_key_types)::value_type& entry){ return entry.first == xml.name(); });
        if (it != m_key_types.cend()) {
            xml_props[it->second] = xml.readElementText();
            continue;
        }

        xml.skipCurrentElement();
    }
    return xml_props;
}

void Metadata::process_gamelist_xml(const QDir& xml_dir, QXmlStreamReader& xml, const providers::SearchContext& sctx) const
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

        const size_t linenum = xml.lineNumber();

        // process node
        HashMap<MetaType, QString, EnumHash> xml_props = parse_gamelist_game_node(xml);
        if (xml_props.empty())
            continue;

        const QString shell_filepath = xml_props[MetaType::PATH];
        if (shell_filepath.isEmpty()) {
            Log::warning(tr_log("%1: The `<game>` node in `%2` at line %3 has no valid `<path>` entry")
                .arg(m_log_tag, static_cast<QFile*>(xml.device())->fileName(), QString::number(linenum)));
            continue;
        }

        // get the Game, if exists, and apply the properties

        const QString filepath = shell_to_canonical_path(xml_dir, shell_filepath);
        if (filepath.isEmpty())  // ie. the file does not exist
            continue;

        model::GameFile* const entry_ptr = sctx.gamefile_by_filepath(filepath);
        if (!entry_ptr)  // ie. the file was not picked up by the system's extension list
            continue;

        apply_metadata(*entry_ptr, xml_dir, xml_props);
    }
    if (xml.error()) {
        Log::warning(tr_log("%1: %2").arg(m_log_tag, xml.errorString()));
        return;
    }
}

void Metadata::find_metadata_for(const SystemEntry& sysentry, const providers::SearchContext& sctx) const
{
    Q_ASSERT(!sysentry.name.isEmpty());
    Q_ASSERT(!sysentry.path.isEmpty());


    if (sysentry.shortname == QLatin1String("steam")) {
        Log::info(tr_log("%1: Ignoring the `steam` system in favor of the built-in Steam support").arg(m_log_tag));
        return;
    }

    const QDir xml_dir(sysentry.path);
    const QString gamelist_path = find_gamelist_xml(m_config_dirs, xml_dir, sysentry.shortname);
    if (gamelist_path.isEmpty()) {
        Log::warning(tr_log("%1: No gamelist file found for system `%2`").arg(m_log_tag, sysentry.shortname));
        return;
    }
    Log::info(tr_log("%1: Found `%2`").arg(m_log_tag, gamelist_path));

    QFile xml_file(gamelist_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        Log::error(tr_log("%1: Could not open `%2`").arg(m_log_tag, gamelist_path));
        return;
    }

    QXmlStreamReader xml(&xml_file);
    process_gamelist_xml(xml_dir, xml, sctx);
}

void Metadata::apply_metadata(model::GameFile& gamefile, const QDir& xml_dir, HashMap<MetaType, QString, EnumHash>& xml_props) const
{
    model::Game& game = *gamefile.parentGame();

    // first, the simple strings
    game.setTitle(xml_props[MetaType::NAME])
        .setDescription(xml_props[MetaType::DESC]);
    game.developerList().append(xml_props[MetaType::DEVELOPER]);
    game.publisherList().append(xml_props[MetaType::PUBLISHER]);
    game.genreList().append(xml_props[MetaType::GENRE]);

    // then the numbers
    const int play_count = xml_props[MetaType::PLAYCOUNT].toInt();
    game.setRating(qBound(0.f, xml_props[MetaType::RATING].toFloat(), 1.f));

    // the player count can be a range
    const QString players_field = xml_props[MetaType::PLAYERS];
    const auto players_match = m_players_regex.match(players_field);
    if (players_match.hasMatch()) {
        short a = 0, b = 0;
        a = players_match.captured(1).toShort();
        b = players_match.captured(3).toShort();
        game.setPlayerCount(std::max(a, b));
    }

    // then the bools
    const QString& favorite_val = xml_props[MetaType::FAVORITE];
    if (favorite_val.compare(QLatin1String("yes"), Qt::CaseInsensitive) == 0
        || favorite_val.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
        || favorite_val.compare(QLatin1String("1")) == 0) {
        game.setFavorite(true);
    }

    // then dates
    // NOTE: QDateTime::fromString returns a null (invalid) date on error

    const QDateTime last_played = QDateTime::fromString(xml_props[MetaType::LASTPLAYED], m_date_format);
    const QDateTime release_time(QDateTime::fromString(xml_props[MetaType::RELEASE], m_date_format));
    game.setReleaseDate(release_time.date());
    gamefile.update_playstats(play_count, 0, last_played);

    // then assets
    // TODO: C++17
    for (const auto& pair : m_asset_type_map) {
        QString path = shell_to_canonical_path(xml_dir, xml_props[pair.first]);
        game.assetsMut().add_file(pair.second, std::move(path));
    }
}

} // namespace es2
} // namespace providers
