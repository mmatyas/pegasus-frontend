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


#include "AndroidAppsProvider.h"

#include "Log.h"
#include "model/gaming/Assets.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"
#include "providers/android_apps/AndroidAppsMetadata.h"

#include <QFileInfo>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>


namespace {
HashMap<QString, model::Game*> find_apps_for(model::Collection& collection, providers::SearchContext& sctx)
{
    constexpr auto JNI_CLASS = "org/pegasus_frontend/android/MainActivity";
    constexpr auto APPLIST_METHOD = "appList";
    constexpr auto APPLIST_SIGNATURE = "()[Lorg/pegasus_frontend/android/App;";
    constexpr auto APP_NAME = "appName";
    constexpr auto APP_PACKAGE = "packageName";
    constexpr auto APP_LAUNCH_ACT = "launchAction";
    constexpr auto APP_LAUNCH_CPT = "launchComponent";


    HashMap<QString, model::Game*> app_game_map;

    QAndroidJniEnvironment jni_env;
    const auto jni_applist_raw = QAndroidJniObject::callStaticObjectMethod(JNI_CLASS, APPLIST_METHOD, APPLIST_SIGNATURE);
    const auto jni_applist = jni_applist_raw.object<jobjectArray>();

    const jsize app_count = jni_env->GetArrayLength(jni_applist);
    for (jsize i = 0; i < app_count; i++) {
        const QAndroidJniObject jni_app = jni_env->GetObjectArrayElement(jni_applist, i);

        const QString appname = jni_app.callObjectMethod<jstring>(APP_NAME).toString();
        const QString package = jni_app.callObjectMethod<jstring>(APP_PACKAGE).toString();
        const QString action = jni_app.callObjectMethod<jstring>(APP_LAUNCH_ACT).toString();
        const QString component = jni_app.callObjectMethod<jstring>(APP_LAUNCH_CPT).toString();

        const QString game_uri = QStringLiteral("android:") + package;
        model::Game* game_ptr = sctx.game_by_uri(game_uri);
        if (!game_ptr) {
            game_ptr = sctx.create_game_for(collection);
            sctx.game_add_uri(*game_ptr, game_uri);
        }
        app_game_map.emplace(package, game_ptr);

        const QString icon_uri = QStringLiteral("image://androidicons/") + package;
        (*game_ptr)
            .setTitle(appname)
            .setLaunchCmd(QStringLiteral("am start --user 0 -a %1 -n %2").arg(action, component))
            .assetsMut()
            .add_uri(AssetType::BOX_FRONT, icon_uri)
            .add_uri(AssetType::UI_TILE, icon_uri);
    }

    return app_game_map;
}


void fill_metadata_from_cache(
    HashMap<QString, model::Game*>& app_game_map,
    const providers::android::MetadataHelper& metahelper)
{
    std::vector<QString> found_apps;
    found_apps.reserve(app_game_map.size());

    // TODO: C++17
    for (const auto& entry : app_game_map) {
        const QString& app = entry.first;
        model::Game& game = *entry.second;

        const bool found = metahelper.fill_from_cache(app, game);
        if (found)
            found_apps.emplace_back(app);
    }

    for (const QString& app : found_apps)
        app_game_map.erase(app);
}

void fill_metadata_from_network(
    HashMap<QString, model::Game*>& app_game_map,
    const providers::android::MetadataHelper& metahelper,
    providers::SearchContext& sctx)
{
    if (app_game_map.empty())
        return;

    if (!sctx.has_network())
        return;

    // TODO: C++17
    for (const auto& entry : app_game_map) {
        const QString& app = entry.first;
        model::Game* const game = entry.second;
        metahelper.fill_from_network(app, *game, sctx);
    }
}
} // namespace


namespace providers {
namespace android {

AndroidAppsProvider::AndroidAppsProvider(QObject* parent)
    : Provider(QLatin1String("androidapps"), QStringLiteral("Android Apps"), parent)
    , m_metahelper(display_name())
{}

Provider& AndroidAppsProvider::run(SearchContext& sctx)
{
    const QString COLLECTION_TAG(QStringLiteral("Android"));
    model::Collection& collection = *sctx.get_or_create_collection(COLLECTION_TAG);
    collection.setShortName(COLLECTION_TAG);

    HashMap<QString, model::Game*> app_game_map = find_apps_for(collection, sctx);
    Log::info(display_name(), LOGMSG("%1 apps found").arg(app_game_map.size()));
    if (app_game_map.empty())
        return *this;

    fill_metadata_from_cache(app_game_map, m_metahelper);
    fill_metadata_from_network(app_game_map, m_metahelper, sctx);
    return *this;
}

} // namespace android
} // namespace providers
