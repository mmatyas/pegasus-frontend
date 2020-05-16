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

#include "model/gaming/Game.h"
#include "providers/SearchContext.h"

#include <QFileInfo>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>


namespace providers {
namespace android {

AndroidAppsProvider::AndroidAppsProvider(QObject* parent)
    : Provider(QLatin1String("androidapps"), QStringLiteral("Android Apps"), PROVIDES_GAMES | PROVIDES_ASSETS, parent)
{}

Provider& AndroidAppsProvider::findLists(SearchContext& sctx)
{
    static constexpr auto JNI_CLASS = "org/pegasus_frontend/android/MainActivity";
    static constexpr auto APPLIST_METHOD = "appList";
    static constexpr auto APPLIST_SIGNATURE = "()[Lorg/pegasus_frontend/android/App;";
    static constexpr auto APP_NAME = "appName";
    static constexpr auto APP_PACKAGE = "packageName";
    static constexpr auto APP_LAUNCH_ACT = "launchAction";
    static constexpr auto APP_LAUNCH_CPT = "launchComponent";


    const QString COLLECTION_TAG(QStringLiteral("Android"));
    PendingCollection& collection = sctx.get_or_create_collection(COLLECTION_TAG);
    collection.inner().setShortName(COLLECTION_TAG);


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

        PendingGame& game = sctx.add_or_create_game_from_entry(package, collection);
        game.inner()
            .setTitle(appname)
            .setLaunchCmd(QStringLiteral("am start --user 0 -a %1 -n %2").arg(action, component));
        game.inner().assets().add_url(AssetType::BOX_FRONT, QStringLiteral("image://androidicons/") + package);
        game.inner().assets().add_url(AssetType::UI_TILE, game.inner().assets().boxFront());
    }

    return *this;
}

Provider& AndroidAppsProvider::findStaticData(SearchContext& sctx)
{
    m_metadata.findStaticData(sctx);
    return *this;
}

} // namespace android
} // namespace providers
