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


#include "GogMetadata.h"

#include "GogCommon.h"
#include "LocaleUtils.h"
#include "providers/JsonCacheUtils.h"
#include "modeldata/gaming/Game.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>


namespace {
static constexpr auto MSG_PREFIX = "GOG:";
static constexpr auto JSON_CACHE_DIR = "gog";

bool read_json(modeldata::Game& game, const QJsonDocument& json)
{
    if (json.isNull())
        return false;

    const auto json_root = json.object();
    if (json_root.isEmpty())
        return false;

    const auto desc = json_root[QLatin1String("description")].toObject();
    if (!desc.isEmpty()) {
        game.summary = desc[QLatin1String("lead")].toString().replace('\n', ' ');
        game.description = desc[QLatin1String("full")].toString().replace('\n', ' ');
    }

    modeldata::GameAssets& assets = game.assets;

    const auto images = json_root[QLatin1String("images")].toObject();
    if (!images.isEmpty()) {
        const QString prefix(QStringLiteral("https:"));
        assets.setSingle(AssetType::BOX_FRONT, prefix + images[QLatin1String("logo2x")].toString());
        assets.setSingle(AssetType::BACKGROUND, prefix + images[QLatin1String("background")].toString());
        assets.setSingle(AssetType::LOGO, prefix + images[QLatin1String("icon")].toString());
    }

    const auto screenshots = json_root[QLatin1String("screenshots")].toArray();
    for (const auto& array_entry : screenshots) {
        const auto screenshot = array_entry.toObject();
        const auto url = screenshot[QLatin1String("formatter_template_url")].toString()
            .replace(QStringLiteral("{formatter}"), QStringLiteral("ggvgm_2x"));

        if (!url.isEmpty())
            assets.appendMulti(AssetType::SCREENSHOTS, url);
    }

    return true;
}

bool fill_from_cache(modeldata::Game& entry)
{
    if (!entry.extra.count(providers::gog::gog_id_key()))
        return false;

    const QString message_prefix = QLatin1String(MSG_PREFIX);
    const QString cache_dir = QLatin1String(JSON_CACHE_DIR);
    const QString entry_name = entry.extra.at(providers::gog::gog_id_key());

    const auto json = providers::read_json_from_cache(message_prefix, cache_dir, entry_name);
    const bool json_success = read_json(entry, json);
    if (!json_success) {
        providers::delete_cached_json(message_prefix, cache_dir, entry_name);
        return false;
    }

    return true;
}

void download_metadata(std::vector<modeldata::Game*>& entries, QNetworkAccessManager& netman)
{
    const int TIMEOUT_MS(5000);
    const auto API_URL(QStringLiteral("https://api.gog.com/products/%1?expand=description,screenshots,videos"));

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
        const QString gog_id = entries[i]->extra.at(providers::gog::gog_id_key());

        const QUrl url(API_URL.arg(gog_id));
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
                           .arg(entries[i]->title, reply->errorString());
                }
                else {
                    const auto raw_data = reply->readAll();
                    const bool json_success = read_json(*entries[i], QJsonDocument::fromJson(raw_data));
                    if (json_success) {
                        providers::cache_json(message_prefix, cache_dir, gog_id, raw_data);
                    }
                    else {
                        qWarning().noquote() << MSG_PREFIX
                            << tr_log("failed to parse the response of the server "
                                      "for game `%1` - perhaps the GOG API changed?")
                                      .arg(entries[i]->title);
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
namespace gog {

Metadata::Metadata(QObject* parent)
    : QObject(parent)
{}

void Metadata::enhance(HashMap<QString, modeldata::Game>& games,
                       const HashMap<QString, modeldata::Collection>&,
                       const HashMap<QString, std::vector<QString>>& collection_childs)
{
    const QString GOG_TAG(QStringLiteral("GOG"));
    if (!collection_childs.count(GOG_TAG))
        return;

    std::vector<modeldata::Game*> entries;

    const std::vector<QString>& childs = collection_childs.at(GOG_TAG);
    for (const QString& game_key : childs) {
        modeldata::Game* game = &games.at(game_key);
        if (game->extra.count(gog_id_key()))
            entries.emplace_back(game);
    }

    // try to fill using cached jsons

    decltype(entries) uncached_entries;
    for (const auto entry : entries) {
        const bool filled = fill_from_cache(*entry);
        if (!filled)
            uncached_entries.push_back(entry);
    }
    if (uncached_entries.empty())
        return;

    // try to fill from network

    QNetworkAccessManager netman; // TODO: move NAM to global
    if (netman.networkAccessible() != QNetworkAccessManager::Accessible) {
        qWarning().noquote()
            << MSG_PREFIX
            << tr_log("no internet connection - most game data may be missing");
        return;
    }
    download_metadata(uncached_entries, netman);
}

} // namespace gog
} // namespace providers
