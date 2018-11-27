// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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


#include "AndroidAppsMetadata.h"

#include "LocaleUtils.h"
#include "modeldata/gaming/CollectionData.h"
#include "modeldata/gaming/GameData.h"
#include "providers/JsonCacheUtils.h"
#include "utils/HashMap.h"

#include <QDebug>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSslSocket>
#include <QTextStream>
#include <QTimer>


namespace {

static constexpr auto MSG_PREFIX = "Android Apps:";
static constexpr auto JSON_CACHE_DIR = "androidapps";

bool read_json(modeldata::Game& game, const QJsonDocument& json)
{
    using Lat = QLatin1String;


    if (json.isNull())
        return false;

    const auto root = json.object();
    if (root.isEmpty())
        return false;

    const QString language = root[Lat("language")].toString();
    if (language != QLocale::system().name())
        return false;


    const QString developer = root[Lat("developer")].toString();
    if (!developer.isEmpty())
        game.developers << root[Lat("developer")].toString();

    game.description = root[Lat("description")].toString();
    game.summary = game.description.section(QChar('\n'), 0, 0);

    const double rating = root[Lat("rating")].toDouble(-1);
    if (0.0 <= rating && rating <= 5.0)
        game.rating = static_cast<float>(rating / 5.0);

    const QString background = root[Lat("background")].toString();
    if (!background.isEmpty())
        game.assets.setSingle(AssetType::BACKGROUND, background);

    const auto sshot_arr = root[Lat("screenshots")].toArray();
    for (const auto& arr_entry : sshot_arr) {
        const auto sshot = arr_entry.toString();
        if (!sshot.isEmpty())
            game.assets.appendMulti(AssetType::SCREENSHOTS, sshot);
    }

    return true;
}

bool fill_from_cached_json(const QString& game_id, modeldata::Game& game)
{
    const QString message_prefix = QLatin1String(MSG_PREFIX);
    const QString cache_dir = QLatin1String(JSON_CACHE_DIR);

    const auto json = providers::read_json_from_cache(message_prefix, cache_dir, game_id);
    if (!read_json(game, json)) {
        providers::delete_cached_json(message_prefix, cache_dir, game_id);
        return false;
    }

    return true;
}

} // namespace


namespace providers {
namespace android {

Metadata::Metadata()
    : rx_meta_itemprops(QStringLiteral(R""(<meta itemprop="(.+?)" content="(.+?)")""), QRegularExpression::DotMatchesEverythingOption)
    , rx_background(QStringLiteral(R""(<meta property="og:image" content="(.+?)")""))
    , rx_developer(QStringLiteral(R""(<a +href="https:\/\/play\.google\.com\/store\/apps\/dev(eloper)?\?id=.+?".*?>([^<]+)<\/a>)""))
    , rx_category(QStringLiteral(R""(<a itemprop="genre".*?>([^<]+)<\/a>)""))
    , rx_screenshots(QStringLiteral(R""(<img src="([^"]+=w720-h310)")""))
{
    rx_meta_itemprops.optimize();
    rx_background.optimize();
    rx_developer.optimize();
    rx_category.optimize();
    rx_screenshots.optimize();
}

void Metadata::findStaticData(HashMap<QString, modeldata::Game>& games,
                              const HashMap<QString, modeldata::Collection>&,
                              const HashMap<QString, std::vector<QString>>& collection_childs)
{
    const auto cc_it = collection_childs.find(QStringLiteral("Android"));
    if (cc_it == collection_childs.cend())
        return;

    const auto uncached_entries = fill_from_cache(cc_it->second, games);
    fill_from_network(uncached_entries, games);
}

std::vector<QString> Metadata::fill_from_cache(const std::vector<QString>& child_ids,
                                               HashMap<QString, modeldata::Game>& all_games)
{
    std::vector<QString> uncached_entries;

    for (auto& child_id : child_ids) {
        modeldata::Game& game = all_games.at(child_id);

        const bool filled = fill_from_cached_json(child_id, game);
        if (!filled)
            uncached_entries.push_back(child_id);
    }

    return uncached_entries;
}

void Metadata::fill_from_network(const std::vector<QString>& child_ids,
                                 HashMap<QString, modeldata::Game>& games)
{
    if (child_ids.empty())
        return;

    if (!QSslSocket::supportsSsl()) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("secure connection (SSL) support not available, downloading metadata is not possible");
        return;
    }

    QNetworkAccessManager netman; // TODO: move NAM to global
    if (netman.networkAccessible() != QNetworkAccessManager::Accessible) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("no internet connection - most game data may be missing");
        return;
    }


    const int TIMEOUT_MS(10000);
    const QString GPLAY_URL(QStringLiteral("https://play.google.com/store/apps/details?id=%1&hl=")
                            + QLocale::system().name());

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

    for (size_t i = 0; i < child_ids.size(); i++) {
        const QString& id = child_ids[i];
        const QUrl url(GPLAY_URL.arg(id));

        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        QNetworkReply* reply = netman.get(request);

        QObject::connect(reply, &QNetworkReply::finished,
            [&, reply](){
                completed_transfers++;
                if (completed_transfers == listeners.count())
                    loop.quit();

                loop_timeout.start(TIMEOUT_MS); // restart the timer


                if (reply->error()) {
                    qWarning().noquote() << MSG_PREFIX
                        << tr_log("downloading metadata for `%1` failed (%2)")
                           .arg(id, reply->errorString());
                    return;
                }


                QJsonObject json;
                QByteArray html_raw = reply->readAll();
                if (parse_reply(html_raw, json)) {
                    const QJsonDocument json_doc(json);
                    modeldata::Game& game = games.at(id);

                    if (read_json(game, json_doc)) {
                        providers::cache_json(message_prefix, cache_dir, id, json_doc.toJson(QJsonDocument::Compact));
                        return;
                    }
                }

                qWarning().noquote() << MSG_PREFIX
                    << tr_log("failed to parse the response of the server "
                              "for app `%1` - perhaps the Google Play sites have changed?")
                              .arg(id);
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

bool Metadata::parse_reply(QByteArray& html_raw, QJsonObject& out_json)
{
    QTextStream html_stream(&html_raw);
    const QString content = html_stream.read(1048576); // up to 1 MB

    {
        HashMap<QString, QString> map;
        auto rx_it = rx_meta_itemprops.globalMatch(content);
        while (rx_it.hasNext()) {
            const auto match = rx_it.next();
            map.emplace(match.captured(1), match.captured(2));
        }

        auto it = map.find(QStringLiteral("description"));
        if (it != map.end())
            out_json.insert(QStringLiteral("description"), it->second);

        it = map.find(QStringLiteral("applicationCategory"));
        if (it != map.end())
            out_json.insert(QStringLiteral("category"), it->second);

        it = map.find(QStringLiteral("image"));
        if (it != map.end())
            out_json.insert(QStringLiteral("icon"), it->second);

        it = map.find(QStringLiteral("rating"));
        if (it != map.end()) {
            bool is_double = false;
            const double val = it->second.toDouble(&is_double);
            if (is_double)
                out_json.insert(QStringLiteral("rating"), val);
        }
    }

    const auto rx_background_match = rx_background.match(content);
    if (rx_background_match.hasMatch())
        out_json.insert(QStringLiteral("background"), rx_background_match.captured(1));

    const auto rx_developer_match = rx_developer.match(content);
    if (rx_developer_match.hasMatch())
        out_json.insert(QStringLiteral("developer"), rx_developer_match.captured(2));

    const auto rx_category_match = rx_developer.match(content);
    if (rx_category_match.hasMatch())
        out_json.insert(QStringLiteral("category"), rx_category_match.captured(1));


    QJsonArray sshot_arr;
    auto rx_screenshots_it = rx_screenshots.globalMatch(content);
    while (rx_screenshots_it.hasNext()) {
        const auto match = rx_screenshots_it.next();
        sshot_arr.append(match.captured(1));
    }
    if (!sshot_arr.isEmpty())
        out_json.insert(QStringLiteral("screenshots"), sshot_arr);


    if (out_json.empty())
        return false;

    out_json.insert(QStringLiteral("query_date"), QDateTime::currentDateTime().toString(Qt::ISODate));
    out_json.insert(QStringLiteral("language"), QLocale::system().name());
    return true;
}

} // namespace android
} // namespace providers
