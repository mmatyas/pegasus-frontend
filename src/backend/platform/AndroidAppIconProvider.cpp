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


#include "AndroidAppIconProvider.h"

#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>


AndroidAppIconProvider::AndroidAppIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

QPixmap AndroidAppIconProvider::requestPixmap(const QString& id, QSize*, const QSize&)
{
    static constexpr auto JNI_CLASS = "org/pegasus_frontend/android/MainActivity";
    static constexpr auto APPICON_METHOD = "appIcon";
    static constexpr auto APPICON_SIGNATURE = "(Ljava/lang/String;)[B";
    static constexpr auto APPICON_FMT = "PNG";

    QAndroidJniEnvironment jni_env;
    const auto jni_packagename = QAndroidJniObject::fromString(id);
    const auto jni_iconbytes_raw = QAndroidJniObject::callStaticObjectMethod(JNI_CLASS,
                                                                             APPICON_METHOD, APPICON_SIGNATURE,
                                                                             jni_packagename.object<jstring>());
    const auto jni_iconbytes_arr = jni_iconbytes_raw.object<jbyteArray>();
    if (!jni_iconbytes_arr)
        return QPixmap();

    const auto jni_iconbytes_len = jni_env->GetArrayLength(jni_iconbytes_arr);
    if (jni_iconbytes_len <= 0)
        return QPixmap();

    jbyte* const jni_iconbytes_p = jni_env->GetByteArrayElements(jni_iconbytes_arr, nullptr);
    if (!jni_iconbytes_p)
        return QPixmap();

    QImage image(QImage::fromData(reinterpret_cast<const uchar*>(jni_iconbytes_p), jni_iconbytes_len, APPICON_FMT));
    jni_env->ReleaseByteArrayElements(jni_iconbytes_arr, jni_iconbytes_p, JNI_ABORT);
    return QPixmap::fromImage(std::move(image));
}
