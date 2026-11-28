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


#pragma once

#include <QString>
#include <QImage>
#include <QByteArray>


namespace android {

const char* jni_classname();

QString primary_storage_path();
QStringList storage_paths();
bool has_external_storage_access();

QStringList granted_paths();
void request_saf_permission(const std::function<void()>&);

QString run_am_call(const QStringList&);
int run_script(const QStringList&);
QString to_content_uri(const QString&);
QString to_document_uri(const QString&);

bool has_secondary_display();
int secondary_width();
int secondary_height();
void secondary_show();
// Returns false if the frame was dropped (the Android side is still busy)
bool secondary_push_frame(const QImage& rgba_premultiplied);
void secondary_hide();

} // namespace android
