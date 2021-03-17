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


#include "LogiqxProvider.h"

#include "Log.h"
#include "providers/SearchContext.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "utils/PathTools.h"

#include <QDirIterator>
#include <QXmlStreamReader>
#include <unordered_set>


namespace {
void log_xml_error(const QString& log_tag, const QString& pretty_path, const QXmlStreamReader& xml)
{
    Q_ASSERT(xml.hasError());
    Log::warning(log_tag, LOGMSG("XML error in `%1` at line %2: %3")
        .arg(pretty_path, QString::number(xml.lineNumber()), xml.errorString()));
}


bool read_datfile_intro(const QString& log_tag, const QString& pretty_path, QXmlStreamReader& xml)
{
    using XmlToken = QXmlStreamReader::TokenType;

    if (xml.readNext() != XmlToken::StartDocument) {
        Log::warning(log_tag, LOGMSG("`%1` doesn't seem to be a valid XML file, ignored").arg(pretty_path));
        return false;
    }
    if (xml.readNext() != XmlToken::DTD) {
        Log::warning(log_tag, LOGMSG("`%1` seems to be a valid XML file, but doesn't have a DOCTYPE declaration, ignored").arg(pretty_path));
        return false;
    }
    if (xml.dtdSystemId() != QLatin1String("http://www.logiqx.com/Dats/datafile.dtd")) {
        Log::warning(log_tag, LOGMSG("`%1` is not declared as a Logiqx XML file, ignored").arg(pretty_path));
        return false;
    }
    if (xml.readNext() != XmlToken::StartElement || xml.name() != QLatin1String("datafile")) {
        Log::warning(log_tag, LOGMSG("`%1` seems to be a Logiqx file, but doesn't start with a `datafile` root element").arg(pretty_path));
        return false;
    }
    if (xml.hasError()) {
        log_xml_error(log_tag, pretty_path, xml);
        return false;
    }

    Log::info(log_tag, LOGMSG("Found `%1`").arg(pretty_path));
    return true;
}


QString read_datfile_header_entry(
    const QString& log_tag, const QString& pretty_path,
    QXmlStreamReader& xml, providers::SearchContext& sctx)
{
    if (!xml.readNextStartElement() || xml.name() != QLatin1String("header")) {
        Log::warning(log_tag, LOGMSG("`%1` does not start with a `header` entry").arg(pretty_path));
        return {};
    }

    QString name;
    QString desc;

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("name")) {
            name = xml.readElementText().trimmed();
            continue;
        }
        if (xml.name() == QLatin1String("description")) {
            desc = xml.readElementText().trimmed();
            continue;
        }
        xml.skipCurrentElement();
    }
    if (xml.hasError()) {
        log_xml_error(log_tag, pretty_path, xml);
        return {};
    }

    if (name.isEmpty()) {
        Log::warning(log_tag, LOGMSG("`%1` has no `name` field in its `header` entry").arg(pretty_path));
        return {};
    }

    model::Collection& collection = *sctx.get_or_create_collection(name);
    if (!desc.isEmpty())
        collection.setDescription(desc);

    return name;
}


void read_datfile_game_entry(
    const QString& log_tag, const QDir& root_dir, const QString& pretty_path,
    QXmlStreamReader& xml,
    model::Collection& collection,
    providers::SearchContext& sctx)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("game"));

    const size_t game_start_linenum = xml.lineNumber();
    const QString name = xml.attributes().value(QLatin1String("name")).trimmed().toString();
    if (name.isEmpty()) {
        Log::warning(log_tag, LOGMSG("The `game` element in `%1` at line %2 has an empty or missing `name` attribute, entry ignored")
            .arg(pretty_path, QString::number(game_start_linenum)));
        xml.skipCurrentElement();
        return;
    }

    QDate release;
    QString description;
    QString manufacturer;
    QStringList rom_paths;

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("year")) {
            bool success = false;
            const unsigned short year = xml.readElementText().toUShort(&success);
            if (success) {
                release = QDate(year, 1, 1);
            } else {
                Log::warning(log_tag, LOGMSG("The `year` element in `%1` at line %2 has an invalid value, ignored")
                    .arg(pretty_path, QString::number(xml.lineNumber())));
            }
            continue;
        }

        if (xml.name() == QLatin1String("description")) {
            description = xml.readElementText().trimmed();
            continue;
        }

        if (xml.name() == QLatin1String("manufacturer")) {
            manufacturer = xml.readElementText().trimmed();
            continue;
        }

        if (xml.name() == QLatin1String("rom")) {
            const QString relpath = xml.attributes().value(QLatin1String("name")).trimmed().toString();
            xml.skipCurrentElement();

            if (relpath.isEmpty()) {
                Log::warning(log_tag, LOGMSG("The `rom` element in `%1` at line %2 has an empty or missing `name` attribute, ignored")
                    .arg(pretty_path, QString::number(xml.lineNumber())));
                continue;
            }

            const QFileInfo finfo(root_dir, relpath);
            if (!finfo.exists()) {
                Log::warning(log_tag, LOGMSG("The `rom` element in `%1` at line %2 refers to file `%3`, which doesn't seem to exist")
                    .arg(pretty_path, QString::number(xml.lineNumber()), ::pretty_path(finfo)));
                continue;
            }

            const QString abs_path = ::clean_abs_path(finfo);
            const auto it = std::find(rom_paths.cbegin(), rom_paths.cend(), abs_path);
            if (it != rom_paths.cend()) {
                Log::warning(log_tag, LOGMSG("The `rom` element in `%1` at line %2 seems to be a duplicate entry, ignored")
                    .arg(pretty_path, QString::number(xml.lineNumber())));
                continue;
            }

            rom_paths.append(abs_path);
            continue;
        }

        xml.skipCurrentElement();
    }

    if (rom_paths.isEmpty()) {
        Log::warning(log_tag, LOGMSG("The `game` element in `%1` at line %2 has no valid `rom` fields, game ignored")
            .arg(pretty_path, QString::number(game_start_linenum)));
        return;
    }

    std::unordered_set<model::Game*> game_ptrs;
    for (const QString& rom_path : rom_paths)
        game_ptrs.emplace(sctx.game_by_filepath(rom_path));
    game_ptrs.erase(nullptr);

    if (game_ptrs.size() > 1) {
        Log::warning(log_tag, LOGMSG(
                "The `game` element in `%1` at line %2 has multiple `rom` fields "
                "that belong to different games; the `game` entry is ignored")
            .arg(pretty_path, QString::number(game_start_linenum)));
        return;
    }

    model::Game& game = game_ptrs.empty()
        ? *sctx.create_game_for(collection)
        : *(*game_ptrs.begin());
    game.setTitle(name);
    if (release.isValid())
        game.setReleaseDate(release);
    if (!manufacturer.isEmpty())
        game.developerList().append(manufacturer);
    if (!description.isEmpty())
        game.setDescription(description);
    for (const QString& rom_path : rom_paths)
        sctx.game_add_filepath(game, rom_path);
}


void read_datfile(const QString& log_tag, const QDir& root_dir, const QString& path, providers::SearchContext& sctx)
{
    const QString pretty_path = QDir::toNativeSeparators(path);

    QFile dat_file(path);
    if (!dat_file.open(QIODevice::ReadOnly)) {
        Log::warning(log_tag, LOGMSG("Could not open `%1`").arg(pretty_path));
        return;
    }

    QXmlStreamReader xml(&dat_file);
    if (!read_datfile_intro(log_tag, pretty_path, xml))
        return;

    const QString coll_name = read_datfile_header_entry(log_tag, pretty_path, xml, sctx);
    if (coll_name.isEmpty())
        return;

    model::Collection& collection = *sctx.get_or_create_collection(coll_name);

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("game")) {
            read_datfile_game_entry(log_tag, root_dir, pretty_path, xml, collection, sctx);
            continue;
        }

        xml.skipCurrentElement();
    }
    if (xml.hasError()) {
        log_xml_error(log_tag, pretty_path, xml);
        return;
    }
}

} // namespace


namespace providers {
namespace logiqx {

LogiqxProvider::LogiqxProvider(QObject* parent)
    : Provider(QLatin1String("logiqx"), QStringLiteral("Logiqx"), parent)
{}


Provider& LogiqxProvider::run(SearchContext& sctx)
{
    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::FollowSymlinks;

    for (const QString& dir_path : sctx.root_game_dirs()) {
        QDirIterator dir_it(dir_path, dir_filters, dir_flags);
        while (dir_it.hasNext()) {
            const QString path = dir_it.next();
            const QFileInfo finfo = dir_it.fileInfo();
            if (finfo.suffix() == QLatin1String("dat"))
                read_datfile(display_name(), finfo.dir(), path, sctx);
        }
    }

    return *this;
}

} // namespace logiqx
} // namespace providers
