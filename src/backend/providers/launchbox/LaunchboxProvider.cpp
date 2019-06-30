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

#include <QDebug>
#include <QDirIterator>
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

void store_game(const QFileInfo& finfo, const HashMap<GameField, QString>&,
                providers::SearchContext& sctx, std::vector<size_t>& collection_childs)
{
    const QString can_path = finfo.canonicalFilePath();

    if (!sctx.path_to_gameid.count(can_path)) {
        modeldata::Game game(finfo);
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
                 providers::SearchContext& sctx)
{
    Q_ASSERT(!lb_dir.isEmpty());
    Q_ASSERT(!xml_path.isEmpty());

    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << MSG_PREFIX << tr_log("could not open `%1`").arg(xml_path);
        return;
    }

    const QString collection_name = QFileInfo(xml_path).baseName();

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
        { QStringLiteral("StarsFloat"), GameField::STARS },
    };
    for (const QString& path : xml_paths)
        process_xml(lb_dir, path, game_field_map, sctx);
}

} // namespace steam
} // namespace providers
