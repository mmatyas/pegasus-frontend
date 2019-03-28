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


#include "AndroidAppsProvider.h"

#include "modeldata/CollectionData.h"
#include "modeldata/GameData.h"

#include <QFileInfo>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>


namespace providers {
namespace android {

AndroidAppsProvider::AndroidAppsProvider(QObject* parent)
    : Provider(parent)
{}

void AndroidAppsProvider::findLists(SearchContext& sctx)
{
    static constexpr auto JNI_CLASS = "org/pegasus_frontend/android/MainActivity";
    static constexpr auto APPLIST_METHOD = "appList";
    static constexpr auto APPLIST_SIGNATURE = "()[Lorg/pegasus_frontend/android/App;";
    static constexpr auto APP_NAME = "appName";
    static constexpr auto APP_PACKAGE = "packageName";
    static constexpr auto APP_LAUNCH_ACT = "launchAction";
    static constexpr auto APP_LAUNCH_CPT = "launchComponent";


    const QString COLLECTION_TAG(QStringLiteral("Android"));
    if (!sctx.collections.count(COLLECTION_TAG))
        sctx.collections.emplace(COLLECTION_TAG, modeldata::Collection(COLLECTION_TAG));

    modeldata::Collection& collection = sctx.collections.at(COLLECTION_TAG);
    std::vector<size_t>& childs = sctx.collection_childs[COLLECTION_TAG];
    collection.setShortName(COLLECTION_TAG);


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

        if (!sctx.path_to_gameid.count(package)) {
            const size_t game_id = sctx.games.size();
            sctx.path_to_gameid.emplace(package, game_id);
            sctx.games.emplace(game_id, modeldata::Game(QFileInfo(package)));
        }

        const size_t game_id = sctx.path_to_gameid.at(package);
        childs.emplace_back(game_id);

        modeldata::Game& game = sctx.games.at(game_id);
        game.title = appname;
        game.launch_cmd = QStringLiteral("am start --user 0 -a %1 -n %2").arg(action, component);

        game.assets.setSingle(AssetType::BOX_FRONT, QStringLiteral("image://androidicons/") + package);
        game.assets.setSingle(AssetType::UI_TILE, game.assets.single(AssetType::BOX_FRONT));
    }
}

void AndroidAppsProvider::findStaticData(SearchContext& sctx)
{
    m_metadata.findStaticData(sctx);
}

} // namespace android
} // namespace providers
