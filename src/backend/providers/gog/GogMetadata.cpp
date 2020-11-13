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


#include "GogMetadata.h"

#include "LocaleUtils.h"
#include "Log.h"
#include "model/gaming/Assets.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/JsonCacheUtils.h"
#include "providers/SearchContext.h"
#include "utils/MoveOnly.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <array>


namespace {

bool apply_api_json(const QString&, model::Game& game, const QJsonDocument& json)
{
    if (json.isNull())
        return false;

    const auto json_root = json.object();
    if (json_root.isEmpty())
        return false;


    const auto desc = json_root[QLatin1String("description")].toObject();
    if (!desc.isEmpty()) {
        game.setSummary(desc[QLatin1String("lead")].toString().replace('\n', ' '))
            .setDescription(desc[QLatin1String("full")].toString().replace('\n', ' '));
    }

    const auto date_raw = json_root[QLatin1String("release_date")].toString();
    game.setReleaseDate(QDate::fromString(date_raw, Qt::ISODate));


    model::Assets& assets = game.assetsMut();  // FIXME: update signals?

    const auto images = json_root[QLatin1String("images")].toObject();
    if (!images.isEmpty()) {
        const QString prefix(QStringLiteral("https:"));

        const QString box_front = images[QLatin1String("logo2x")].toString();
        const QString background = images[QLatin1String("background")].toString();
        const QString logo = images[QLatin1String("icon")].toString();

        if (!box_front.isEmpty())
            assets.add_uri(AssetType::BOX_FRONT, prefix + box_front);
        if (!background.isEmpty())
            assets.add_uri(AssetType::BACKGROUND, prefix + background);
        if (!logo.isEmpty())
            assets.add_uri(AssetType::LOGO, prefix + logo);
    }

    const auto screenshots = json_root[QLatin1String("screenshots")].toArray();
    for (const auto& array_entry : screenshots) {
        const auto screenshot = array_entry.toObject();
        const auto url = screenshot[QLatin1String("formatter_template_url")].toString()
            .replace(QStringLiteral("{formatter}"), QStringLiteral("ggvgm_2x"));

        if (!url.isEmpty())
            assets.add_uri(AssetType::SCREENSHOT, url);
    }

    return true;
}

bool apply_embed_json(const QString& gogid, model::Game& game, const QJsonDocument& json)
{
    if (json.isNull())
        return false;

    const auto json_root = json.object();
    if (json_root.isEmpty())
        return false;


    const auto products = json_root[QLatin1String("products")].toArray();
    for (const auto& products_entry : products) {
        const auto product = products_entry.toObject();

        const auto id = product[QLatin1String("id")].toInt();
        if (id == 0 || QString::number(id) != gogid)
            continue;

        game.developerList().append(product[QLatin1String("developer")].toString());
        game.publisherList().append(product[QLatin1String("publisher")].toString());

        const auto genres = product[QLatin1String("genres")].toArray();
        for (const auto& genres_entry : genres)
            game.genreList().append(genres_entry.toString());
    }

    return true;
}
} // namespace


namespace providers {
namespace gog {

Metadata::Metadata(QString log_tag)
    : m_log_tag(std::move(log_tag))
    , m_json_cache_dir(QStringLiteral("gog"))
    , m_json_api_suffix(QStringLiteral("_api"))
    , m_json_embed_suffix(QStringLiteral("_embed"))
{}

bool Metadata::fill_from_cache(const QString& gogid, model::Game& game) const
{
    const QString entry_api = gogid + m_json_api_suffix;
    const QString entry_embed = gogid + m_json_embed_suffix;

    const auto json_api = providers::read_json_from_cache(m_log_tag, m_json_cache_dir, entry_api);
    const bool json_api_success = apply_api_json(gogid, game, json_api);
    if (!json_api_success)
        providers::delete_cached_json(m_log_tag, m_json_cache_dir, entry_api);

    const auto json_embed = providers::read_json_from_cache(m_log_tag, m_json_cache_dir, entry_embed);
    const bool json_embed_success = apply_embed_json(gogid, game, json_embed);
    if (!json_embed_success)
        providers::delete_cached_json(m_log_tag, m_json_cache_dir, entry_embed);

    return json_api_success && json_embed_success;
}

void Metadata::fill_from_network(const QString& gogid, model::Game& game, SearchContext& sctx) const
{
    const QString api_url_str = QStringLiteral("https://api.gog.com/products/%1?expand=description,screenshots,videos").arg(gogid);
    const QString embed_url_str = QStringLiteral("https://embed.gog.com/games/ajax/filtered?mediaType=game&search=%1").arg(gogid);
    const QUrl api_url(api_url_str, QUrl::StrictMode);
    const QUrl embed_url(embed_url_str, QUrl::StrictMode);
    Q_ASSERT(api_url.isValid());
    Q_ASSERT(embed_url.isValid());
    if (Q_UNLIKELY(!api_url.isValid() || !embed_url.isValid()))
        return;

    // TODO: C++17
    using JsonCallback = std::function<bool(const QString&, model::Game&, const QJsonDocument&)>;
    const std::array<std::tuple<QUrl, QString, JsonCallback>, 2> requests {
        std::make_tuple(api_url, m_json_api_suffix, apply_api_json),
        std::make_tuple(embed_url, m_json_embed_suffix, apply_embed_json),
    };

    // TODO: C++17
    model::Game* const game_ptr = &game;
    QString log_tag = m_log_tag;
    QString json_cache_dir = m_json_cache_dir;
    for (const auto& triplet : requests) {
        const QString json_suffix = std::get<1>(triplet);
        const JsonCallback& json_callback = std::get<2>(triplet);
        sctx.schedule_download(std::get<0>(triplet), [log_tag, json_cache_dir, gogid, game_ptr, json_suffix, json_callback](QNetworkReply* const reply){
            if (reply->error()) {
                Log::warning(tr_log("%1: Downloading metadata for `%2` failed: %3")
                    .arg(log_tag, game_ptr->title(), reply->errorString()));
                return;
            }

            const QByteArray raw_data = reply->readAll();
            const QJsonDocument json = QJsonDocument::fromJson(raw_data);
            if (json.isNull()) {
                Log::warning(tr_log(
                       "%1: Failed to parse the response of the server for game '%2', "
                       "either it's no longer available from the GOG Store or the GOG API has changed"
                   ).arg(log_tag, game_ptr->title()));
                return;
            }

            const bool success = json_callback(gogid, *game_ptr, json);
            if (success) {
                const QString json_name = gogid + json_suffix;
                providers::cache_json(log_tag, json_cache_dir, json_name, json.toJson(QJsonDocument::Compact));
            }
        });
    }
}

} // namespace gog
} // namespace providers
