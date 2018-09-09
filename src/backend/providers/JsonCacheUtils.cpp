// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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


#include "JsonCacheUtils.h"

#include "Paths.h"
#include "LocaleUtils.h"

#include <QDebug>
#include <QDir>
#include <QStringBuilder>


namespace {
QString cached_json_path(const QString& provider_prefix,
                         const QString& provider_dir,
                         const QString& entryname)
{
    Q_ASSERT(!paths::writableCacheDir().isEmpty()); // according to the Qt docs

    const QString cache_path = paths::writableCacheDir() % '/' % provider_dir;

    // NOTE: mkpath() returns true if the dir already exists
    QDir cache_dir(cache_path);
    if (!cache_dir.mkpath(QStringLiteral("."))) {
        qWarning().noquote()
            << provider_prefix
            << tr_log("could not create cache directory `%1`").arg(cache_path);
        return QString();
    }

    return cache_path % QLatin1Char('/') % entryname % QLatin1String(".json");
}
} // namespace


namespace providers {

void cache_json(const QString& provider_prefix,
                const QString& provider_dir,
                const QString& entryname,
                const QByteArray& bytes)
{
    const QString json_path = cached_json_path(provider_prefix, provider_dir, entryname);

    QFile json_file(json_path);
    if (!json_file.open(QIODevice::WriteOnly)) {
        qWarning().noquote()
            << provider_prefix
            << tr_log("could not create cache file `%1`").arg(json_path);
        return;
    }

    if (json_file.write(bytes) != bytes.length()) {
        qWarning().noquote()
            << provider_prefix
            << tr_log("writing cache file `%1` failed").arg(json_path);
        json_file.remove();
    }
}

QJsonDocument read_json_from_cache(const QString& provider_prefix,
                                   const QString& provider_dir,
                                   const QString& entryname)
{
    const QString json_path = cached_json_path(provider_prefix, provider_dir, entryname);

    QFile json_file(json_path);
    if (!json_file.open(QIODevice::ReadOnly))
        return {};

    QJsonParseError parse_result;
    auto json = QJsonDocument::fromJson(json_file.readAll(), &parse_result);
    if (parse_result.error != QJsonParseError::NoError) {
        qDebug()
            << provider_prefix
            << tr_log("could not parse cached file `%1`").arg(json_path)
            << parse_result.errorString();
        json_file.remove();
        return {};
    }

    return json;
}

void delete_cached_json(const QString& provider_prefix,
                        const QString& provider_dir,
                        const QString& entryname)
{
    const QString json_path = cached_json_path(provider_prefix, provider_dir, entryname);

    QFile::remove(json_path);
}

} // namespace providers
