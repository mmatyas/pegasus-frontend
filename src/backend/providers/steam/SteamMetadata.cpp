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

#include "LocaleUtils.h"
#include "Paths.h"
#include "types/gaming/Collection.h"

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
#include <QSettings>
#include <QStringBuilder>
#include <QTimer>


namespace {
static constexpr auto MSG_PREFIX = "Steam:";

struct SteamGameEntry {
    QString title;
    QString appid;
    Types::Game* game_ptr;

    SteamGameEntry() : game_ptr(nullptr) {}

    bool parsed() const { return !title.isEmpty() && !appid.isEmpty(); }
};

QString find_steam_exe()
{
#ifdef Q_OS_WIN
    QSettings reg_base(QLatin1String("HKEY_CURRENT_USER\\Software\\Valve\\Steam"),
                       QSettings::NativeFormat);
    QString reg_value = reg_base.value(QLatin1String("SteamExe")).toString();
    if (!reg_value.isEmpty())
        return reg_value.prepend('"').append('"');
#endif
    // it should be in the PATH
    return QStringLiteral("steam");
}

SteamGameEntry read_manifest(const QString& manifest_path)
{
    QFile manifest(manifest_path);
    if (!manifest.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning().noquote() << MSG_PREFIX << tr_log("could not open `%1`").arg(manifest_path);
        return {};
    }

    using regex = QRegularExpression;
    static const regex appid_regex(QStringLiteral(R""("appid"\s+"(\d+)")""), regex::CaseInsensitiveOption);
    static const regex title_regex(QStringLiteral(R""("name"\s+"([^"]+)")""));

    SteamGameEntry entry;

    QTextStream stream(&manifest);
    while (!stream.atEnd() && !entry.parsed()) {
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

bool read_json(Types::Game& game, const QByteArray& bytes)
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

    Types::GameAssets& assets = game.assets();

    game.m_title = app_data[QLatin1String("name")].toString();

    game.m_summary = app_data[QLatin1String("short_description")].toString();
    game.m_description = app_data[QLatin1String("about_the_game")].toString();

    const auto reldate_obj = app_data[QLatin1String("release_date")].toObject();
    if (!reldate_obj.isEmpty()) {
        const QString date_str = reldate_obj[QLatin1String("date")].toString();

        // FIXME: the date format will likely fail for non-English locales (see Qt docs)
        const QDateTime datetime(QDateTime::fromString(date_str, QLatin1String("d MMM, yyyy")));
        if (datetime.isValid())
            game.setRelease(datetime.date());
    }

    const QString header_image = app_data[QLatin1String("header_image")].toString();
    assets.setSingle(AssetType::LOGO, header_image);
    assets.setSingle(AssetType::UI_STEAMGRID, header_image);
    assets.setSingle(AssetType::BOX_FRONT, header_image);

    const QJsonArray developer_arr = app_data[QLatin1String("developers")].toArray();
    if (!developer_arr.isEmpty()) {
        for (int i = 0; i < developer_arr.count(); i++)
            game.addDeveloper(developer_arr[i].toString());
    }
    const QJsonArray publisher_arr = app_data[QLatin1String("publishers")].toArray();
    if (!publisher_arr.isEmpty()) {
        for (int i = 0; i < publisher_arr.count(); i++)
            game.addPublisher(publisher_arr[i].toString());
    }

    const auto metacritic_obj = app_data[QLatin1String("metacritic")].toObject();
    if (!metacritic_obj.isEmpty()) {
        const double score = metacritic_obj[QLatin1String("score")].toDouble(-1);
        if (0.0 <= score && score <= 100.0)
            game.m_rating = static_cast<float>(score / 100.0);
    }

    const auto genre_arr = app_data[QLatin1String("genres")].toArray();
    for (const auto& arr_entry : genre_arr) {
        const auto genre_obj = arr_entry.toObject();
        if (genre_obj.isEmpty())
            break; // assume the rest will fail too

        const QString genre = genre_obj[QLatin1String("description")].toString();
        if (!genre.isEmpty())
            game.addGenre(genre);
    }

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
    auto cache_path = paths::writableCacheDir();
    Q_ASSERT(!cache_path.isEmpty()); // according to the Qt docs

    cache_path += QLatin1String("/steam");
    QDir cache_dir(cache_path);
    if (!cache_dir.mkpath(QLatin1String("."))) {
        // NOTE: mkpath() returns true if the dir already exists
        qWarning().noquote() << MSG_PREFIX
            << tr_log("could not create cache directory `%1`").arg(cache_path);
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
            << tr_log("could not create cache file `%1` for game `%2`")
               .arg(json_path, entry.title);
        return;
    }

    if (json_file.write(bytes) != bytes.length()) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("writing cache file `%1` was not successful")
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
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        QNetworkReply* reply = netman.get(request);

        QObject::connect(reply, &QNetworkReply::finished,
            [&, i, reply](){
                completed_transfers++;
                if (completed_transfers == listeners.count())
                    loop.quit();


                if (reply->error()) {
                    qWarning().noquote() << MSG_PREFIX
                        << tr_log("downloading metadata for `%1` failed (%2)")
                           .arg(entries[i].title, reply->errorString());
                }
                else {
                    const auto raw_data = reply->readAll();
                    const bool json_success = read_json(*entries[i].game_ptr, raw_data);
                    if (json_success) {
                        cache_json(entries[i], raw_data);
                    }
                    else {
                        qWarning().noquote() << MSG_PREFIX
                            << tr_log("failed to parse the response of the server "
                                      "for game `%1` - perhaps the Steam API changed?")
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


namespace providers {
namespace steam {

Metadata::Metadata(QObject* parent)
    : QObject(parent)
{}

void Metadata::enhance(const QHash<QString, Types::Game*>&,
                       const QHash<QString, Types::Collection*>& collections)
{
    const QString STEAM_TAG(QStringLiteral("Steam"));
    if (!collections.contains(STEAM_TAG))
        return;

    const Types::Collection* const& collection = collections[STEAM_TAG];
    const QString steamexe = find_steam_exe();

    // try to fill using manifest files

    std::vector<SteamGameEntry> entries;

    for (Types::Game* const game_ptr : collection->gameList().allGames()) {
        Q_ASSERT(game_ptr);
        SteamGameEntry entry = read_manifest(game_ptr->m_fileinfo.filePath());
        if (!entry.appid.isEmpty()) {
            if (entry.title.isEmpty())
                entry.title = QLatin1String("App #") % entry.appid;

            game_ptr->m_title = entry.title;
            game_ptr->m_launch_cmd = steamexe % QLatin1String(" steam://rungameid/") % entry.appid;
            entry.game_ptr = game_ptr;

            entries.push_back(std::move(entry));
        }
    }

    if (entries.empty()) {
        qInfo().noquote() << MSG_PREFIX << tr_log("couldn't find any installed games");
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
            << tr_log("no internet connection - most game data may be missing");
        return;
    }

    download_metadata(uncached_entries, netman);
}

} // namespace steam
} // namespace providers
