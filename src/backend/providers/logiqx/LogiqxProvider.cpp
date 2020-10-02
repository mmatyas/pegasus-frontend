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

#include "AppSettings.h"
#include "LocaleUtils.h"
#include "Log.h"
#include "providers/SearchContext.h"
#include "model/gaming/Collection.h"

#include <QDirIterator>
#include <QXmlStreamReader>


namespace {
QStringList read_game_dirs()
{
    QStringList game_dirs;

    AppSettings::parse_gamedirs([&game_dirs](const QString& line){
        const QFileInfo finfo(line);
        if (finfo.isDir())
            game_dirs.append(finfo.canonicalFilePath());
    });

    game_dirs.removeDuplicates();
    return game_dirs;
}


void log_xml_error(const QString& log_tag, const QString& pretty_path, const QXmlStreamReader& xml)
{
    Q_ASSERT(xml.hasError());
    qWarning().noquote() << QStringLiteral("%1: XML error in `%2` at line %3: %4")
        .arg(log_tag, pretty_path, QString::number(xml.lineNumber()), xml.errorString());
}


bool read_datfile_intro(const QString& log_tag, const QString& pretty_path, QXmlStreamReader& xml)
{
    using XmlToken = QXmlStreamReader::TokenType;

    if (xml.readNext() != XmlToken::StartDocument) {
        qInfo().noquote() << tr_log("%1: `%2` doesn't seem to be a valid XML file, ignored").arg(log_tag, pretty_path);
        return false;
    }
    if (xml.readNext() != XmlToken::DTD) {
        qInfo().noquote() << tr_log("%1: `%2` seems to be a valid XML file, but doesn't have a DOCTYPE declaration, ignored").arg(log_tag, pretty_path);
        return false;
    }
    if (xml.dtdSystemId() != QLatin1String("http://www.logiqx.com/Dats/datafile.dtd")) {
        qInfo().noquote() << tr_log("%1: `%2` is not declared as a Logiqx XML file, ignored").arg(log_tag, pretty_path);
        return false;
    }
    if (xml.readNext() != XmlToken::StartElement || xml.name() != QLatin1String("datafile")) {
        qWarning().noquote() << tr_log("%1: `%2` seems to be a Logiqx file, but doesn't start with a `datafile` root element").arg(log_tag, pretty_path);
        return false;
    }
    if (xml.hasError()) {
        log_xml_error(log_tag, pretty_path, xml);
        return false;
    }

    qInfo().noquote() << tr_log("%1: Found `%2`").arg(log_tag, pretty_path);
    return true;
}


QString read_datfile_header_entry(
    const QString& log_tag, const QString& pretty_path,
    QXmlStreamReader& xml, providers::SearchContext& sctx)
{
    if (!xml.readNextStartElement() || xml.name() != QLatin1String("header")) {
        qWarning().noquote() << tr_log("%1: `%2` does not start with a `header` entry").arg(log_tag, pretty_path);
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
        qWarning().noquote() << tr_log("%1: `%2` has no `name` field in its `header` entry").arg(log_tag, pretty_path);
        return {};
    }

    providers::PendingCollection& collection = sctx.get_or_create_collection(name);
    if (!desc.isEmpty())
        collection.inner().setDescription(desc);

    return name;
}


void read_datfile_game_entry(
    const QString& log_tag, const QDir& root_dir, const QString& pretty_path,
    QXmlStreamReader& xml,
    providers::PendingCollection& collection,
    providers::SearchContext& sctx)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("game"));

    const QString name = xml.attributes().value(QLatin1String("name")).trimmed().toString();
    if (name.isEmpty()) {
        qWarning().noquote() << tr_log("%1: The `game` element in `%2` at line %3 has an empty or missing `name` attribute, entry ignored")
            .arg(log_tag, pretty_path, QString::number(xml.lineNumber()));
        xml.skipCurrentElement();
        return;
    }

    providers::PendingGame& game = sctx.create_bare_game_for(name, &collection);

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("year")) {
            bool success = false;
            const unsigned short year = xml.readElementText().toUShort(&success);
            if (success) {
                game.inner().setReleaseDate(QDate(year, 1, 1));
            } else {
                qWarning().noquote() << tr_log("%1: The `year` element in `%2` at line %3 has an invalid value, ignored")
                    .arg(log_tag, pretty_path, QString::number(xml.lineNumber()));
            }
            continue;
        }

        if (xml.name() == QLatin1String("description")) {
            const QString desc = xml.readElementText().trimmed();
            if (!desc.isEmpty())
                game.inner().setDescription(desc);
            continue;
        }

        if (xml.name() == QLatin1String("manufacturer")) {
            const QString manufacturer = xml.readElementText().trimmed();
            if (!manufacturer.isEmpty())
                game.inner().developerList().append(manufacturer);
            continue;
        }

        if (xml.name() == QLatin1String("rom")) {
            const QString relpath = xml.attributes().value(QLatin1String("name")).trimmed().toString();
            xml.skipCurrentElement();

            if (relpath.isEmpty()) {
                qWarning().noquote() << tr_log("%1: The `rom` element in `%2` at line %3 has an empty or missing `name` attribute, ignored")
                    .arg(log_tag, pretty_path, QString::number(xml.lineNumber()));
                continue;
            }

            const QFileInfo finfo(root_dir, relpath);
            const QString can_path = finfo.canonicalFilePath();
            if (can_path.isEmpty() || !finfo.exists()) {
                qWarning().noquote() << tr_log("%1: The `rom` element in `%2` at line %3 refers to file `%4`, which doesn't seem to exist")
                    .arg(log_tag, pretty_path, QString::number(xml.lineNumber()), finfo.absoluteFilePath());
                continue;
            }

            const bool already_registered = std::any_of(game.files().cbegin(), game.files().cend(),
                [&finfo](const model::GameFile* const file){ return file->fileinfo() == finfo; });
            if (already_registered) {
                qWarning().noquote() << tr_log("%1: The `rom` element in `%2` at line %3 seems to be a duplicate entry, ignored")
                    .arg(log_tag, pretty_path, QString::number(xml.lineNumber()));
                continue;
            }

            sctx.create_game_file_for(finfo, game);
            continue;
        }

        xml.skipCurrentElement();
    }
}


void read_datfile(const QString& log_tag, const QDir& root_dir, const QString& path, providers::SearchContext& sctx)
{
    const QString pretty_path = QDir::toNativeSeparators(path);

    QFile dat_file(path);
    if (!dat_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << tr_log("%1: Could not open `%2`").arg(log_tag, pretty_path);
        return;
    }

    QXmlStreamReader xml(&dat_file);
    if (!read_datfile_intro(log_tag, pretty_path, xml))
        return;

    const QString coll_name = read_datfile_header_entry(log_tag, pretty_path, xml, sctx);
    if (coll_name.isEmpty())
        return;

    providers::PendingCollection& collection = sctx.get_or_create_collection(coll_name);

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
    : Provider(QLatin1String("logiqx"), QStringLiteral("Logiqx"), PROVIDES_GAMES, parent)
{}


Provider& LogiqxProvider::load() {
    return load_with_config(read_game_dirs());
}

Provider& LogiqxProvider::load_with_config(const QStringList& game_dirs) {
    m_game_dirs = game_dirs;
    return *this;
}

Provider& LogiqxProvider::unload() {
    m_game_dirs.clear();
    return *this;
}


Provider& LogiqxProvider::findLists(SearchContext& sctx)
{
    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::FollowSymlinks;

    for (const QString& dir_path : m_game_dirs) {
        QDirIterator dir_it(dir_path, dir_filters, dir_flags);
        while (dir_it.hasNext()) {
            const QString path = dir_it.next();
            const QFileInfo finfo = dir_it.fileInfo();
            if (finfo.suffix() == QLatin1String("dat"))
                read_datfile(Provider::name(), finfo.dir(), path, sctx);
        }
    }

    return *this;
}

} // namespace logiqx
} // namespace providers
