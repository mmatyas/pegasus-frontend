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
#include "configfiles/FavoriteDB.h" // FIXME
#include "model/Filters.h"
#include "model/Meta.h"
#include "model/System.h"
#include "model/gaming/CollectionList.h"
#include "model/settings/Settings.h"

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

    Q_PROPERTY(model::Filters* filters READ filters CONSTANT)
    Q_PROPERTY(model::Meta* meta READ meta CONSTANT)
    Q_PROPERTY(model::Settings* settings READ settings CONSTANT)
    Q_PROPERTY(model::System* system READ system CONSTANT)
    Q_PROPERTY(model::CollectionList* collectionList READ collectionList CONSTANT)

    // shortcuts

    Q_PROPERTY(model::Collection* currentCollection
               READ currentCollection
               NOTIFY currentCollectionChanged)
    Q_PROPERTY(model::Game* currentGame
               READ currentGame
               NOTIFY currentGameChanged)

    // retranslate on locale change
    Q_PROPERTY(QString tr READ emptyString NOTIFY localeChanged)

public:
    explicit ApiObject(QObject* parent = nullptr);

    // scanning
    void startScanning();

    // subcomponents
    model::Filters* filters() { return &m_filters; }
    model::Meta* meta() { return &m_meta; }
    model::Settings* settings() { return &m_settings; }
    model::System* system() { return &m_system; }
    model::CollectionList* collectionList() { return &m_collections; }

    // shortcuts

    model::Collection* currentCollection() const { return m_collections.current(); }
    model::Game* currentGame() const {
        return currentCollection() ? currentCollection()->gameList().current() : nullptr;
    }

signals:
    // game launching
    void prepareLaunch();
    void executeLaunch(const modeldata::Collection* const, const modeldata::Game* const);
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
    void onScanComplete();
    void onLaunchRequested(const modeldata::Collection* const, const modeldata::Game* const);
    void onFiltersChanged();
    void onGameFavoriteChanged();

private:
    model::Meta m_meta;
    model::System m_system;
    model::Settings m_settings;
    model::Filters m_filters;
    model::CollectionList m_collections;

    // the collection and game that requested the game launch
    const modeldata::Collection* m_launch_collection;
    const modeldata::Game* m_launch_game;

    // initialization
    DataFinder m_datafinder;
    QFutureWatcher<void> m_loading_watcher;

    // favorite management
    FavoriteWriter m_favorite_writer;

    // used to trigger re-rendering of texts on locale change
    QString emptyString() const { return QString(); }

    // model data
    std::vector<modeldata::Collection> m_gaming_data;
};
