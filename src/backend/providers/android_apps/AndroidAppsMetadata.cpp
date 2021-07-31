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


#include "AndroidAppsMetadata.h"

#include "Log.h"
#include "model/gaming/Assets.h"
#include "model/gaming/Game.h"
#include "providers/ProviderUtils.h"
#include "providers/SearchContext.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QTextStream>


namespace {

bool apply_json(model::Game& game, const QJsonDocument& json)
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
        game.developerList().append(root[Lat("developer")].toString());

    game.setDescription(root[Lat("description")].toString());
    game.setSummary(game.description().section(QChar('\n'), 0, 0));

    const double rating = root[Lat("rating")].toDouble(-1);
    if (rating > 0.0)
        game.setRating(rating / 5.0);

    const QString background = root[Lat("background")].toString();
    if (!background.isEmpty())
        game.assetsMut().add_uri(AssetType::BACKGROUND, background);

    const auto sshot_arr = root[Lat("screenshots")].toArray();
    for (const auto& arr_entry : sshot_arr) {
        const auto sshot = arr_entry.toString();
        if (!sshot.isEmpty())
            game.assetsMut().add_uri(AssetType::SCREENSHOT, sshot);
    }

    return true;
}

} // namespace


namespace providers {
namespace android {

MetadataHelper::MetadataHelper(QString log_tag)
    : m_log_tag(std::move(log_tag))
    , m_json_cache_dir(QStringLiteral("androidapps"))
    , rx_meta_itemprops(QStringLiteral(R""(<meta itemprop="(.+?)" content="(.+?)")""), QRegularExpression::DotMatchesEverythingOption)
    , rx_background(QStringLiteral(R""(<meta property="og:image" content="(.+?)")""))
    , rx_developer(QStringLiteral(R""(<a +href="https:\/\/play\.google\.com\/store\/apps\/dev(eloper)?\?id=.+?".*?>([^<]+)<\/a>)""))
    , rx_category(QStringLiteral(R""(<a itemprop="genre".*?>([^<]+)<\/a>)""))
    , rx_screenshots(QStringLiteral(R""(<img src="([^"]+=w720-h310)")""))
{}

bool MetadataHelper::fill_from_cache(const QString& app_package, model::Game& game) const
{
    const auto json = providers::read_json_from_cache(m_log_tag, m_json_cache_dir, app_package);
    const bool success = apply_json(game, json);
    if (!success)
        providers::delete_cached_json(m_log_tag, m_json_cache_dir, app_package);

    return success;
}

void MetadataHelper::fill_from_network(
    const QString& app_package,
    model::Game& game,
    SearchContext& sctx) const
{
    const QString url_str = QStringLiteral("https://play.google.com/store/apps/details?id=%1&hl=%2")
        .arg(app_package, QLocale::system().name());
    const QUrl url(url_str, QUrl::StrictMode);
    Q_ASSERT(url.isValid());
    if (Q_UNLIKELY(!url.isValid()))
        return;


    model::Game* const game_ptr = &game;
    sctx.schedule_download(url, [this, app_package, game_ptr](QNetworkReply* const reply){
        if (reply->error()) {
            Log::warning(m_log_tag, LOGMSG("Downloading metadata for `%1` failed: %2")
               .arg(app_package, reply->errorString()));
            return;
        }

        const QByteArray html_raw = reply->readAll();
        const QJsonDocument json = parse_reply(html_raw);
        if (json.isNull()) {
            Log::warning(m_log_tag, LOGMSG(
                   "Failed to parse the response of the server for app `%1`: "
                   "either it's not available from the Play Store, or the site has changed"
               ).arg(app_package));
            return;
        }

        const bool success = apply_json(*game_ptr, json);
        if (success)
            providers::cache_json(m_log_tag, m_json_cache_dir, app_package, json.toJson(QJsonDocument::Compact));
    });
}

QJsonDocument MetadataHelper::parse_reply(const QByteArray& html_raw) const
{
    QTextStream html_stream(html_raw);
    const QString content = html_stream.read(1 * 1024 * 1024); // up to 1 MB

    QJsonObject json_root;

    {
        HashMap<QString, QString> map;
        auto rx_it = rx_meta_itemprops.globalMatch(content);
        while (rx_it.hasNext()) {
            const auto match = rx_it.next();
            map.emplace(match.captured(1), match.captured(2));
        }

        auto it = map.find(QStringLiteral("description"));
        if (it != map.end())
            json_root.insert(QStringLiteral("description"), it->second);

        it = map.find(QStringLiteral("applicationCategory"));
        if (it != map.end())
            json_root.insert(QStringLiteral("category"), it->second);

        it = map.find(QStringLiteral("image"));
        if (it != map.end())
            json_root.insert(QStringLiteral("icon"), it->second);

        it = map.find(QStringLiteral("rating"));
        if (it != map.end()) {
            bool is_double = false;
            const double val = it->second.toDouble(&is_double);
            if (is_double)
                json_root.insert(QStringLiteral("rating"), val);
        }
    }

    const auto rx_background_match = rx_background.match(content);
    if (rx_background_match.hasMatch())
        json_root.insert(QStringLiteral("background"), rx_background_match.captured(1));

    const auto rx_developer_match = rx_developer.match(content);
    if (rx_developer_match.hasMatch())
        json_root.insert(QStringLiteral("developer"), rx_developer_match.captured(2));

    const auto rx_category_match = rx_developer.match(content);
    if (rx_category_match.hasMatch())
        json_root.insert(QStringLiteral("category"), rx_category_match.captured(1));


    QJsonArray sshot_arr;
    auto rx_screenshots_it = rx_screenshots.globalMatch(content);
    while (rx_screenshots_it.hasNext()) {
        const auto match = rx_screenshots_it.next();
        sshot_arr.append(match.captured(1));
    }
    if (!sshot_arr.isEmpty())
        json_root.insert(QStringLiteral("screenshots"), sshot_arr);


    if (json_root.empty())
        return {};

    json_root.insert(QStringLiteral("query_date"), QDateTime::currentDateTime().toString(Qt::ISODate));
    json_root.insert(QStringLiteral("language"), QLocale::system().name());
    return QJsonDocument(std::move(json_root));
}

} // namespace android
} // namespace providers
