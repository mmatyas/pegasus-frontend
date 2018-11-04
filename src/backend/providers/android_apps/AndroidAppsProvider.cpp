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


#include "AndroidAppsProvider.h"

#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"

#include <QFileInfo>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>


namespace providers {
namespace android {

AndroidAppsProvider::AndroidAppsProvider(QObject* parent)
    : Provider(parent)
{}

void AndroidAppsProvider::findLists(HashMap<QString, modeldata::Game>& games,
                                    HashMap<QString, modeldata::Collection>& collections,
                                    HashMap<QString, std::vector<QString>>& collection_childs)
{
    static constexpr auto JNI_CLASS = "org/pegasus_frontend/android/MainActivity";
    static constexpr auto APPLIST_METHOD = "appList";
    static constexpr auto APPLIST_SIGNATURE = "()[Lorg/pegasus_frontend/android/App;";
    static constexpr auto APP_NAME = "appName";
    static constexpr auto APP_PACKAGE = "packageName";
    static constexpr auto APP_LAUNCH_ACT = "launchAction";
    static constexpr auto APP_LAUNCH_CPT = "launchComponent";


    const QString COLLECTION_TAG(QStringLiteral("Android"));
    if (!collections.count(COLLECTION_TAG))
        collections.emplace(COLLECTION_TAG, modeldata::Collection(COLLECTION_TAG));

    modeldata::Collection& collection = collections.at(COLLECTION_TAG);
    std::vector<QString>& childs = collection_childs[COLLECTION_TAG];
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

        childs.emplace_back(package);
        if (!games.count(package))
            games.emplace(package, modeldata::Game(QFileInfo(package)));

        modeldata::Game& game = games.at(package);
        game.title = appname;
        game.launch_cmd = QStringLiteral("am start --user 0 -a %1 -n %2").arg(action, component);

        game.assets.setSingle(AssetType::BOX_FRONT, QStringLiteral("image://androidicons/") + package);
        game.assets.setSingle(AssetType::UI_TILE, game.assets.single(AssetType::BOX_FRONT));
    }
}

void AndroidAppsProvider::findStaticData(HashMap<QString, modeldata::Game>& games,
                                         const HashMap<QString, modeldata::Collection>& collections,
                                         const HashMap<QString, std::vector<QString>>& collection_childs)
{
    m_metadata.findStaticData(games, collections, collection_childs);
}

} // namespace android
} // namespace providers
