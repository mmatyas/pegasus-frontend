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


#include "SteamMetadata.h"

#include "LocaleUtils.h"
#include "Paths.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/JsonCacheUtils.h"
#include "providers/SearchContext.h"
#include "utils/CommandTokenizer.h"

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
static constexpr auto JSON_CACHE_DIR = "steam";

QString find_steam_call()
{
#if defined(Q_OS_WIN)
    QSettings reg_base(QLatin1String("HKEY_CURRENT_USER\\Software\\Valve\\Steam"),
                       QSettings::NativeFormat);
    QString reg_value = reg_base.value(QLatin1String("SteamExe")).toString();
    if (!reg_value.isEmpty())
        return ::utils::escape_command(reg_value);
#endif

#if defined(Q_OS_MACOS)
    // it should be installed
    return QStringLiteral("open -a Steam --args");
#else
    // it should be in the PATH
    return QStringLiteral("steam");
#endif
}

struct SteamGameEntry {
    QString title;
    QString appid;
    model::Game* game_ptr;
};

SteamGameEntry read_manifest_file(QFile& file, const providers::PendingGame& game)
{
    using regex = QRegularExpression;
    static const regex appid_regex(QStringLiteral(R""("appid"\s+"(\d+)")""), regex::CaseInsensitiveOption);
    static const regex title_regex(QStringLiteral(R""("name"\s+"([^"]+)")""));

    QString appid;
    QString title;

    QTextStream stream(&file);
    while (!stream.atEnd() && (appid.isEmpty() || title.isEmpty())) {
        const QString line = stream.readLine();

        const auto appid_match = appid_regex.match(line);
        if (appid_match.hasMatch()) {
            appid = appid_match.captured(1);
            continue;
        }

        const auto title_match = title_regex.match(line);
        if (title_match.hasMatch()) {
            title = title_match.captured(1);
        }
    }

    return SteamGameEntry { title, appid, game.ptr() };
}

bool read_json(model::Game& game, const QJsonDocument& json)
{
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

    model::Assets& assets = game.assets();

    game.setTitle(app_data[QLatin1String("name")].toString())
        .setSummary(app_data[QLatin1String("short_description")].toString())
        .setDescription(app_data[QLatin1String("about_the_game")].toString());

    const auto reldate_obj = app_data[QLatin1String("release_date")].toObject();
    if (!reldate_obj.isEmpty()) {
        const QString date_str = reldate_obj[QLatin1String("date")].toString();

        // FIXME: the date format will likely fail for non-English locales (see Qt docs)
        const QDateTime datetime(QDateTime::fromString(date_str, QLatin1String("d MMM, yyyy")));
        if (datetime.isValid())
            game.setReleaseDate(datetime.date());
    }

    const QString header_image = app_data[QLatin1String("header_image")].toString();
    assets.add_url(AssetType::LOGO, header_image);
    assets.add_url(AssetType::UI_STEAMGRID, header_image);
    assets.add_url(AssetType::BOX_FRONT, header_image);

    const QJsonArray developer_arr = app_data[QLatin1String("developers")].toArray();
    for (const auto& arr_entry : developer_arr)
        game.developerList().append(arr_entry.toString());

    const QJsonArray publisher_arr = app_data[QLatin1String("publishers")].toArray();
    for (const auto& arr_entry : publisher_arr)
        game.publisherList().append(arr_entry.toString());

    const auto metacritic_obj = app_data[QLatin1String("metacritic")].toObject();
    if (!metacritic_obj.isEmpty()) {
        const double score = metacritic_obj[QLatin1String("score")].toDouble(-1);
        if (0.0 <= score && score <= 100.0)
            game.setRating(static_cast<float>(score / 100.0));
    }

    const auto genre_arr = app_data[QLatin1String("genres")].toArray();
    for (const auto& arr_entry : genre_arr) {
        const auto genre_obj = arr_entry.toObject();
        if (genre_obj.isEmpty())
            break; // assume the rest will fail too

        const QString genre = genre_obj[QLatin1String("description")].toString();
        if (!genre.isEmpty())
            game.genreList().append(genre);
    }

    const auto category_arr = app_data[QLatin1String("categories")].toArray();
    for (const auto& arr_entry : category_arr) {
        const auto cat_obj = arr_entry.toObject();
        if (cat_obj.isEmpty())
            break; // assume the rest will fail too

        const QString category = cat_obj[QLatin1String("description")].toString();
        if (!category.isEmpty())
            game.tagList().append(category);
    }

    const QString background_image = app_data[QLatin1String("background")].toString();
    if (!background_image.isEmpty())
        assets.add_url(AssetType::BACKGROUND, background_image);

    const auto screenshots_arr = app_data[QLatin1String("screenshots")].toArray();
    for (const auto& arr_entry : screenshots_arr) {
        const auto screenshot_obj = arr_entry.toObject();
        if (screenshot_obj.isEmpty())
            break; // assume the rest will fail too

        const QString thumb_path = screenshot_obj[QLatin1String("path_thumbnail")].toString();
        if (!thumb_path.isEmpty())
            assets.add_url(AssetType::SCREENSHOT, thumb_path);
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
            assets.add_url(AssetType::VIDEO, p480_path);
    }

    return true;
}

bool fill_from_cache(const SteamGameEntry& entry)
{
    const QString message_prefix = QLatin1String(MSG_PREFIX);
    const QString cache_dir = QLatin1String(JSON_CACHE_DIR);

    const auto json = providers::read_json_from_cache(message_prefix, cache_dir, entry.appid);
    const bool json_success = read_json(*entry.game_ptr, json);
    if (!json_success) {
        providers::delete_cached_json(message_prefix, cache_dir, entry.appid);
        return false;
    }

    return true;
}

void download_metadata(const std::vector<SteamGameEntry>& entries, QNetworkAccessManager& netman)
{
    const int TIMEOUT_MS(5000);
    const QString APPDETAILS_URL(QLatin1String("https://store.steampowered.com/api/appdetails/?appids="));

    const QString message_prefix = QLatin1String(MSG_PREFIX);
    const QString cache_dir = QLatin1String(JSON_CACHE_DIR);


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
                    const bool json_success = read_json(*entries[i].game_ptr, QJsonDocument::fromJson(raw_data));
                    if (json_success) {
                        providers::cache_json(message_prefix, cache_dir, entries[i].appid, raw_data);
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

void Metadata::enhance(providers::SearchContext& sctx)
{
    const QString STEAM_TAG(QStringLiteral("Steam"));

    const auto slot = sctx.collections().find(STEAM_TAG);
    if (slot == sctx.collections().end())
        return;

    const PendingCollection& coll = slot->second;
    const QString steam_call = find_steam_call();

    // try to fill using manifest files

    std::vector<SteamGameEntry> entries;

    for (const size_t game_id : coll.game_ids()) {
        const PendingGame& game = sctx.games().at(game_id);

        // Steam games can have only one manifest file
        Q_ASSERT(game.files().size() == 1);
        const QString manifest_path = game.files().front()->fileinfo().absoluteFilePath();

        QFile manifest(manifest_path);
        if (!manifest.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning().noquote() << MSG_PREFIX << tr_log("could not open `%1`").arg(manifest_path);
            continue;
        }

        SteamGameEntry entry = read_manifest_file(manifest, game);
        if (entry.appid.isEmpty())
            continue;

        if (entry.title.isEmpty())
            entry.title = QLatin1String("App #") % entry.appid;

        game.inner().setTitle(entry.title);
        game.inner().setSortBy(entry.title);
        game.inner().setLaunchCmd(steam_call % QLatin1String(" steam://rungameid/") % entry.appid);

        entries.push_back(std::move(entry));
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
    download_metadata(uncached_entries, netman);
}

} // namespace steam
} // namespace providers
