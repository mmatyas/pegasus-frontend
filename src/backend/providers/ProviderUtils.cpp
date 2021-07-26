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


#include "ProviderUtils.h"

#include "Log.h"
#include "Paths.h"
#include "utils/CommandTokenizer.h"

#include <QDir>
#include <QSettings>
#include <QStringBuilder>


namespace {
#ifdef Q_OS_LINUX
const QLatin1String STEAM_FLATPAK_PKG("com.valvesoftware.Steam");
#endif // Q_OS_LINUX

#ifdef WITH_JSON_CACHE
QString cached_json_path(const QString& provider_prefix,
                         const QString& provider_dir,
                         const QString& entryname)
{
    Q_ASSERT(!paths::writableCacheDir().isEmpty()); // according to the Qt docs

    const QString cache_path = paths::writableCacheDir() % '/' % provider_dir;

    // NOTE: mkpath() returns true if the dir already exists
    QDir cache_dir(cache_path);
    if (!cache_dir.mkpath(QStringLiteral("."))) {
        Log::warning(provider_prefix,
            LOGMSG("Could not create cache directory `%1`").arg(cache_path));
        return QString();
    }

    return cache_path % QLatin1Char('/') % entryname % QLatin1String(".json");
}
#endif // WITH_JSON_CACHE
} // namespace


namespace providers {

#ifdef WITH_JSON_CACHE
void cache_json(const QString& provider_prefix,
                const QString& provider_dir,
                const QString& entryname,
                const QByteArray& bytes)
{
    const QString json_path = cached_json_path(provider_prefix, provider_dir, entryname);

    QFile json_file(json_path);
    if (!json_file.open(QIODevice::WriteOnly)) {
        Log::warning(provider_prefix,
            LOGMSG("Could not create cache file `%1`").arg(json_path));
        return;
    }

    if (json_file.write(bytes) != bytes.length()) {
        Log::warning(provider_prefix,
            LOGMSG("Writing cache file `%1` failed").arg(json_path));
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

    QJsonParseError parse_result {};
    auto json = QJsonDocument::fromJson(json_file.readAll(), &parse_result);
    if (parse_result.error != QJsonParseError::NoError) {
        Log::warning(provider_prefix, LOGMSG("Could not parse cached file `%1`: %2")
            .arg(json_path, parse_result.errorString()));
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
#endif // WITH_JSON_CACHE


#ifdef Q_OS_LINUX
QString steam_flatpak_data_dir() {
    return paths::homePath()
        % QLatin1String("/.var/app/")
        % STEAM_FLATPAK_PKG
        % QLatin1String("/data/Steam/");
}
#endif // Q_OS_LINUX

QString find_steam_call()
{
#ifdef Q_OS_WIN
    QSettings reg_base(QStringLiteral("HKEY_CURRENT_USER\\Software\\Valve\\Steam"), QSettings::NativeFormat);
    QString reg_value = reg_base.value(QLatin1String("SteamExe")).toString();
    if (!reg_value.isEmpty())
        return ::utils::escape_command(reg_value);
#endif

#ifdef Q_OS_LINUX
    // Assume the Flatpak version exists if its data directory is present
    const QString flatpak_data_dir = steam_flatpak_data_dir();
    if (QFileInfo::exists(flatpak_data_dir))
        return QLatin1String("flatpak run ") % STEAM_FLATPAK_PKG;
#endif

#ifdef Q_OS_MACOS
    // it should be installed
    return QStringLiteral("open -a Steam --args");
#else
    // it should be in the PATH
    return QStringLiteral("steam");
#endif
}

} // namespace providers
