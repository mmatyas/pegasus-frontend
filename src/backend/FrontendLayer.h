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


#pragma once

#include <QObject>
#include <QQmlApplicationEngine>

#ifdef Q_OS_ANDROID
#include "platform/AndroidAppIconProvider.h"
#endif


/// Manages the dynamic reload of the frontend layer
///
/// When we launch a game, the frontend stack will be teared down to save
/// resources. However, this happens asyncronously (see QObject destructor).
/// When it's done, the relevant signal will be triggered. After the actual
/// execution is finished, the frontend layer can be rebuilt again.
///
/// Some funtions require a pointer to the API object, to connect and make
/// it accessible to the frontend.
class FrontendLayer : public QObject {
    Q_OBJECT

public:
    explicit FrontendLayer(QObject* const api, QObject* parent = nullptr);

    void rebuild();
    void teardown();

    void clearCache();

signals:
    void rebuildComplete();
    void teardownComplete();

private:
    QObject* const m_api;
    QQmlApplicationEngine* m_engine;

#ifdef Q_OS_ANDROID
    AndroidAppIconProvider m_android_icon_provider;
#endif
};
