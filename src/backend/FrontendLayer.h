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
#include <QPointer>
#include <QQmlApplicationEngine>


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
    explicit FrontendLayer(QObject* parent = nullptr);

    void rebuild(QObject* api);
    void teardown();

signals:
    void rebuildComplete();
    void teardownComplete();

private:
    QPointer<QQmlApplicationEngine> engine;
};
