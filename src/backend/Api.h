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


#pragma once

#include "model/ApiInternal.h"
#include "model/gaming/Collection.h"
#include "model/general/Keys.h"
#include "providers/ProviderManager.h"

#include <QObject>
#include <QQmlListProperty>


#define API_MEMBER(type, name) \
    public: \
        type* name##Ptr() { return &m_##name; } \
        type& name() { return m_##name; } \
    private: \
        type m_##name; \
        Q_PROPERTY(type* name READ name##Ptr CONSTANT)


/// Provides data access for QML
///
/// Provides an API for the frontend layer, to allow accessing every public
/// property of the backend from QML.
class ApiObject : public QObject {
    Q_OBJECT

    API_MEMBER(model::ApiInternal, internal)
    API_MEMBER(model::Keys, keys)
    Q_PROPERTY(QQmlListProperty<model::Collection> collections READ collections NOTIFY collectionsChanged)

    // retranslate on locale change
    Q_PROPERTY(QString tr READ emptyString NOTIFY localeChanged)

public:
    explicit ApiObject(QObject* parent = nullptr);

    // scanning
    void startScanning();

signals:
    void collectionsChanged();

    // game launching
    void launchGame(const model::Game*);

    // triggers translation update
    void localeChanged();

public slots:
    // game launch communication
    void onGameFinished();
    void onGameLaunchOk();
    void onGameLaunchError();

private slots:
    // internal communication
    void onStaticDataLoaded();
    void onGameFavoriteChanged();
    void onGameLaunchRequested();

private:
    QQmlListProperty<model::Collection> collections();

    QVector<model::Collection*> m_collections;
    QVector<model::Game*> m_all_games_data;

    // game launching
    model::Game* m_launch_game;

    // initialization
    ProviderManager m_providerman;

    // used to trigger re-rendering of texts on locale change
    QString emptyString() const { return QString(); }
};
