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

#include "AppCloseType.h"
#include "DataFinder.h"
#include "FavoriteDB.h"
#include "types/Filters.h"
#include "types/Meta.h"
#include "types/System.h"
#include "types/gaming/CollectionList.h"
#include "types/settings/Settings.h"

#include <QFutureWatcher>
#include <QObject>
#include <QQmlListProperty>


/// Provides data access for QML
///
/// Provides an API for the frontend layer, to allow accessing every public
/// property of the backend from QML.
class ApiObject : public QObject {
    Q_OBJECT

    // subcomponents

    Q_PROPERTY(types::Filters* filters READ filters CONSTANT)
    Q_PROPERTY(types::Meta* meta READ meta CONSTANT)
    Q_PROPERTY(types::Settings* settings READ settings CONSTANT)
    Q_PROPERTY(types::System* system READ system CONSTANT)
    Q_PROPERTY(types::CollectionList* collectionList READ collectionList CONSTANT)

    // shortcuts

    Q_PROPERTY(types::Collection* currentCollection
               READ currentCollection
               NOTIFY currentCollectionChanged)
    Q_PROPERTY(types::Game* currentGame
               READ currentGame
               NOTIFY currentGameChanged)

    // retranslate on locale change
    Q_PROPERTY(QString tr READ emptyString NOTIFY localeChanged)

public:
    explicit ApiObject(QObject* parent = nullptr);

    // scanning
    void startScanning();

    // subcomponents
    types::Filters* filters() { return &m_filters; }
    types::Meta* meta() { return &m_meta; }
    types::Settings* settings() { return &m_settings; }
    types::System* system() { return &m_system; }
    types::CollectionList* collectionList() { return &m_collections; }

    // shortcuts

    types::Collection* currentCollection() const { return m_collections.current(); }
    types::Game* currentGame() const {
        return currentCollection() ? currentCollection()->gameList().current() : nullptr;
    }

signals:
    // game launching
    void prepareLaunch();
    void executeLaunch(const types::Collection*, const types::Game*);
    void restoreAfterGame(ApiObject*);

    // triggers translation update
    void localeChanged();

    // quit/reboot/shutdown
    void appCloseRequested(AppCloseType);

    // shortcuts
    void currentCollectionChanged();
    void currentGameChanged();

    // partial QML reload
    void qmlClearCacheRequested();

public slots:
    // game launch communication
    void onReadyToLaunch();
    void onGameFinished();

private slots:
    // internal communication
    void onLaunchRequested(const types::Collection*, const types::Game*);
    void onFiltersChanged();
    void onGameFavoriteChanged();

private:
    types::Meta m_meta;
    types::System m_system;
    types::Settings m_settings;
    types::Filters m_filters;
    types::CollectionList m_collections;

    // the collection and game that requested the game launch
    const types::Collection* m_launch_collection;
    const types::Game* m_launch_game;

    // initialization
    DataFinder m_datafinder;
    QFutureWatcher<void> m_loading_watcher;

    // favorite management
    FavoriteDB m_favorite_db;

    // used to trigger re-rendering of texts on locale change
    QString emptyString() const { return QString(); }
};
