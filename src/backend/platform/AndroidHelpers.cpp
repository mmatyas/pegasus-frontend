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


#include "AndroidHelpers.h"

#include <QHash>
#include <QStandardPaths>
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>


namespace android {

constexpr const char* jni_classname() {
    return "org/pegasus_frontend/android/MainActivity";
}

QString primary_storage_path()
{
    // should be the same as storage_paths().front(), but perhaps faster
    return QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).constFirst();
}

std::vector<QString> storage_paths()
{
    static constexpr auto JNI_METHOD = "sdcardPaths";
    static constexpr auto JNI_SIGNATURE = "()[Ljava/lang/String;";

    QAndroidJniEnvironment jni_env;
    const auto jni_path_arr_raw = QAndroidJniObject::callStaticObjectMethod(jni_classname(), JNI_METHOD, JNI_SIGNATURE);
    const auto jni_path_arr = jni_path_arr_raw.object<jobjectArray>();
    const jsize path_count = jni_env->GetArrayLength(jni_path_arr);

    std::vector<QString> out;
    out.reserve(static_cast<size_t>(path_count));

    for (jsize i = 0; i < path_count; i++) {
        const auto jni_path_raw = QAndroidJniObject(jni_env->GetObjectArrayElement(jni_path_arr, i));
        out.emplace_back(jni_path_raw.toString());
    }

    if (out.empty())
        out.emplace_back(primary_storage_path());

    return out;
}

bool has_external_storage_access()
{
    using namespace QtAndroid;


    // Android 11
    if (androidSdkVersion() >= 30) {
        static constexpr auto JNI_METHOD = "getAllStorageAccess";
        const auto has_permission = QAndroidJniObject::callStaticMethod<jboolean>(jni_classname(), JNI_METHOD);
        if (!has_permission)
            return false;
    }


    const QStringList required_permissions {
        QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE"),
    };

    const bool has_all_permissions = std::all_of(
        required_permissions.cbegin(),
        required_permissions.cend(),
        [](const QString& p){ return checkPermission(p) == PermissionResult::Granted; });
    if (has_all_permissions)
        return true;

    const PermissionResultMap granted_permissions = requestPermissionsSync(required_permissions);
    for (const QString& p : required_permissions) {
        const PermissionResult result = granted_permissions.value(p, PermissionResult::Denied);
        if (result != PermissionResult::Granted)
            return false;
    }

    return true;
}

} // namespace android
