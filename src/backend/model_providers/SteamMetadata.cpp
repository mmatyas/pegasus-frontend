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


#include "SteamMetadata.h"

#include "model/Platform.h"

#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QTimer>

static constexpr auto MSG_PREFIX = "Steam:";


namespace {

struct SteamGameEntry {
    QString title;
    QString appid;
    Model::Game* game_ptr;

    SteamGameEntry() : game_ptr(nullptr) {}

    bool valid() const { return !title.isEmpty() && !appid.isEmpty(); }
};

SteamGameEntry read_manifest(const QString& manifest_path)
{
    QFile manifest(manifest_path);
    if (!manifest.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning().noquote() << MSG_PREFIX << QObject::tr("could not open `%1`").arg(manifest_path);
        return {};
    }

    using regex = QRegularExpression;
    static const regex appid_regex(R""("appid"\s+"(\d+)")"", regex::CaseInsensitiveOption);
    static const regex title_regex(R""("name"\s+"([^"]+)")"");

    SteamGameEntry entry;

    QTextStream stream(&manifest);
    while (!stream.atEnd() && !entry.valid()) {
        const QString line = stream.readLine();

        const auto appid_match = appid_regex.match(line);
        if (appid_match.hasMatch()) {
            entry.appid = appid_match.captured(1);
            continue;
        }

        const auto title_match = title_regex.match(line);
        if (title_match.hasMatch()) {
            entry.title = title_match.captured(1);
        }
    }

    return entry;
}

QString join_json_array(const QJsonArray& arr)
{
    if (arr.isEmpty())
        return QString();

    QString result;
    for (int i = 0; i < arr.count() - 1; i++) {
        result += arr[i].toString() % ", ";
    }
    result += arr.last().toString();

    return result;
}

bool read_json(Model::Game& game, const QByteArray& bytes)
{
    const auto json = QJsonDocument::fromJson(bytes);
    if (json.isNull())
        return false;

    const auto json_root = json.object();
    if (json_root.isEmpty())
        return false;

    const auto app_entry = json_root.begin().value().toObject();
    if (app_entry.isEmpty())
        return false;

    const bool app_success = app_entry[QLatin1String("success")].toBool();
    if (!app_success)
        return false;

    const auto app_data = app_entry[QLatin1String("data")].toObject();
    if (app_data.isEmpty())
        return false;

    // now the actual field reading

    Model::GameAssets& assets = *game.assets();

    game.m_description = app_data[QLatin1String("short_description")].toString();
    if (game.m_description.isEmpty())
        game.m_description = app_data[QLatin1String("about_the_game")].toString();

    const auto reldate_obj = app_data[QLatin1String("release_date")].toObject();
    if (!reldate_obj.isEmpty()) {
        const QString date_str = reldate_obj[QLatin1String("date")].toString();

        // FIXME: the date format will likely fail for non-English locales (see Qt docs)
        const QDateTime datetime(QDateTime::fromString(date_str, QLatin1String("d MMM, yyyy")));
        if (datetime.isValid()) {
            const QDate date(datetime.date());
            game.m_year = date.year();
            game.m_month = date.month();
            game.m_day = date.day();
        }
    }

    const QString header_image = app_data[QLatin1String("header_image")].toString();
    assets.setSingle(AssetType::LOGO, header_image);
    assets.setSingle(AssetType::STEAMGRID, header_image);
    assets.setSingle(AssetType::BOX_FRONT, header_image);

    game.m_developer = join_json_array(app_data[QLatin1String("developers")].toArray());
    game.m_publisher = join_json_array(app_data[QLatin1String("publishers")].toArray());

    const QString background_image = app_data[QLatin1String("background")].toString();
    if (!background_image.isEmpty())
        assets.setSingle(AssetType::BACKGROUND, background_image);

    const auto screenshots_arr = app_data[QLatin1String("screenshots")].toArray();
    for (const auto& arr_entry : screenshots_arr) {
        const auto screenshot_obj = arr_entry.toObject();
        if (screenshot_obj.isEmpty())
            break; // assume the rest will fail too

        const QString thumb_path = screenshot_obj[QLatin1String("path_thumbnail")].toString();
        if (!thumb_path.isEmpty())
            assets.appendMulti(AssetType::SCREENSHOTS, thumb_path);
    }

    const auto movies_arr = app_data[QLatin1String("movies")].toArray();
    for (const auto& arr_entry : movies_arr) {
        const auto movie_obj = arr_entry.toObject();
        if (movie_obj.isEmpty())
            break;

        const auto webm_obj = movie_obj[QLatin1String("webm")].toObject();
        if (webm_obj.isEmpty())
            break;

        const QString p480_path = webm_obj[QLatin1String("480")].toString();
        if (!p480_path.isEmpty())
            assets.appendMulti(AssetType::VIDEOS, p480_path);
    }

    return true;
}

QString cached_json_path(const SteamGameEntry& entry)
{
    auto cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    Q_ASSERT(!cache_path.isEmpty()); // according to the Qt docs

    cache_path += QLatin1String("/steam");
    QDir cache_dir(cache_path);
    if (!cache_dir.mkpath(QLatin1String("."))) {
        // NOTE: mkpath() returns true if the dir already exists
        qWarning().noquote() << MSG_PREFIX
            << QObject::tr("could not create cache directory `%1`").arg(cache_path);
        return QString();
    }

    return cache_path % QLatin1Char('/') % entry.appid % QLatin1String(".json");
}

void cache_json(const SteamGameEntry& entry, const QByteArray& bytes)
{
    const QString json_path = cached_json_path(entry);
    QFile json_file(json_path);
    if (!json_file.open(QIODevice::WriteOnly)) {
        qWarning().noquote() << MSG_PREFIX
            << QObject::tr("could not create cache file `%1` for game `%2`")
               .arg(json_path, entry.title);
        return;
    }

    if (json_file.write(bytes) != bytes.length()) {
        qWarning().noquote() << MSG_PREFIX
            << QObject::tr("writing cache file `%1` was not successful")
               .arg(json_path, entry.title);
        json_file.remove();
    }
}

bool fill_from_cache(const SteamGameEntry& entry)
{
    const QString json_path = cached_json_path(entry);
    QFile json_file(json_path);
    if (!json_file.open(QIODevice::ReadOnly))
        return false;

    const bool json_success = read_json(*entry.game_ptr, json_file.readAll());
    if (!json_success) {
        json_file.remove();
        return false;
    }

    return true;
}

void download_metadata(const std::vector<SteamGameEntry>& entries, QNetworkAccessManager& netman)
{
    const int TIMEOUT_MS(10000);
    const QString APPDETAILS_URL(QLatin1String("http://store.steampowered.com/api/appdetails/?appids="));

    QVector<QNetworkReply*> listeners;
    int completed_transfers = 0;

    QTimer loop_timeout;
    loop_timeout.setSingleShot(true);
    loop_timeout.start(TIMEOUT_MS);

    QEventLoop loop;
    QObject::connect(&loop_timeout, &QTimer::timeout,
                     &loop, &QEventLoop::quit);

    for (size_t i = 0; i < entries.size(); i++) {
        const QUrl url(APPDETAILS_URL + entries[i].appid);
        QNetworkReply* reply = netman.get(QNetworkRequest(url));

        QObject::connect(reply, &QNetworkReply::finished,
            [&, i, reply](){
                completed_transfers++;
                if (completed_transfers == listeners.count())
                    loop.quit();


                if (reply->error()) {
                    qWarning().noquote() << MSG_PREFIX
                        << QObject::tr("downloading metadata for `%1` failed")
                           .arg(entries[i].title);
                }
                else {
                    const auto raw_data = reply->readAll();
                    const bool json_success = read_json(*entries[i].game_ptr, raw_data);
                    if (json_success) {
                        cache_json(entries[i], raw_data);
                    }
                    else {
                        qWarning().noquote() << MSG_PREFIX
                            << QObject::tr("failed to parse the response of the server "
                                           "for game `%` - perhaps the Steam API changed?")
                                           .arg(entries[i].title);
                    }
                }

                loop_timeout.start(TIMEOUT_MS); // restart the timer
            });

        listeners << reply;
    }

    loop.exec();


    for (auto& listener : listeners) {
        if (!listener->isFinished())
            listener->abort();

        listener->deleteLater();
        listener = nullptr;
    }
}

} // namespace


namespace model_providers {

void SteamMetadata::fill(const Model::Platform& platform)
{
    if (platform.m_short_name != QLatin1Literal("steam"))
        return;

    // try to fill using manifest files

    std::vector<SteamGameEntry> entries;

    for (Model::Game* const game_ptr : platform.allGames()) {
        Q_ASSERT(game_ptr);
        SteamGameEntry entry = read_manifest(game_ptr->m_rom_path);
        if (entry.valid()) {
            game_ptr->m_title = entry.title;
            game_ptr->m_launch_cmd = QLatin1String("steam steam://rungameid/") % entry.appid;
            entry.game_ptr = game_ptr;

            entries.push_back(std::move(entry));
        }
    }

    if (entries.empty()) {
        qInfo().noquote() << MSG_PREFIX
            << QObject::tr("couldn't find any installed games");
        return;
    }

    // try to fill using cached jsons

    std::vector<SteamGameEntry> uncached_entries;
    for (auto& entry : entries) {
        const bool filled = fill_from_cache(entry);
        if (!filled)
            uncached_entries.push_back(std::move(entry));
    }

    if (uncached_entries.empty())
        return;

    // try to fill from network

    QNetworkAccessManager netman; // TODO: move NAM to global
    if (netman.networkAccessible() != QNetworkAccessManager::Accessible) {
        qWarning().noquote() << MSG_PREFIX
            << QObject::tr("no internet connection - most game data may be missing");
        return;
    }

    download_metadata(uncached_entries, netman);
}

} // namespace model_providers
