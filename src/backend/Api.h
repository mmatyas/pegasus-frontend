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

#include "model/gaming/CollectionList.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Filters.h"
#include "model/general/Keys.h"
#include "model/general/Meta.h"
#include "model/general/System.h"
#include "model/settings/Settings.h"
#include "providers/ProviderManager.h"
#include "types/AppCloseType.h"

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
    Q_PROPERTY(model::Keys* keys READ keys CONSTANT)
    Q_PROPERTY(model::CollectionList* collections READ collections CONSTANT)

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

public:
    // subcomponents
    model::Filters* filters() { return &m_filters; }
    model::Meta* meta() { return &m_meta; }
    model::Settings* settings() { return &m_settings; }
    model::System* system() { return &m_system; }
    model::Keys* keys() { return &m_keys; }
    model::CollectionList* collections() { return &m_collections; }

    // shortcuts
    model::Collection* currentCollection() const { return m_collections.current(); }
    model::Game* currentGame() const {
        return currentCollection() ? currentCollection()->gameList().current() : nullptr;
    }

signals:
    // game launching
    void launchGame(const model::Collection*, const model::Game*);

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
    void onGameFinished();
    void onGameLaunchOk();
    void onGameLaunchError();

private slots:
    // internal communication

    void onStaticDataLoaded(QVector<model::Collection*>, QVector<model::Game*>);

    void onLaunchRequested(model::Collection*, model::Game*);
    void onFiltersChanged();
    void onGameFavoriteChanged();

private:
    model::Meta m_meta;
    model::System m_system;
    model::Settings m_settings;
    model::Filters m_filters;
    model::Keys m_keys;
    model::CollectionList m_collections;

    // game launching
    model::Collection* m_launch_collection;
    model::Game* m_launch_game;

    // initialization
    ProviderManager m_providerman;

    // used to trigger re-rendering of texts on locale change
    QString emptyString() const { return QString(); }
};
