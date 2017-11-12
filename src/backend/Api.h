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
#include "types/Filters.h"
#include "types/Meta.h"
#include "types/Settings.h"
#include "types/System.h"
#include "types/CollectionList.h"

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

    Q_PROPERTY(Types::Filters* filters READ filters CONSTANT)
    Q_PROPERTY(Types::Meta* meta READ meta CONSTANT)
    Q_PROPERTY(Types::Settings* settings READ settings CONSTANT)
    Q_PROPERTY(Types::System* system READ system CONSTANT)
    Q_PROPERTY(Types::CollectionList* collections READ collectionList CONSTANT)

    // shortcuts

    Q_PROPERTY(Types::Collection* currentCollection
               READ currentCollection
               NOTIFY currentCollectionChanged)
    Q_PROPERTY(Types::Game* currentGame
               READ currentGame
               NOTIFY currentGameChanged)

    // retranslate on locale change
    Q_PROPERTY(QString tr READ emptyString NOTIFY localeChanged)

public:
    explicit ApiObject(QObject* parent = nullptr);

    // game launching
    Q_INVOKABLE void launchGame();

    // scanning
    void startScanning();

    // subcomponents
    Types::Filters* filters() { return &m_filters; }
    Types::Meta* meta() { return &m_meta; }
    Types::Settings* settings() { return &m_settings; }
    Types::System* system() { return &m_system; }
    Types::CollectionList* collectionList() { return &m_collections; }

    // shortcuts

    Types::Collection* currentCollection() const { return m_collections.current(); }
    Types::Game* currentGame() const {
        return currentCollection() ? currentCollection()->gameList().current() : nullptr;
    }

signals:
    // game launching
    void prepareLaunch();
    void executeLaunch(const Types::Collection*, const Types::Game*);
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
    void onFiltersChanged();

private:
    Types::Meta m_meta;
    Types::System m_system;
    Types::Settings m_settings;
    Types::Filters m_filters;
    Types::CollectionList m_collections;

    // initialization
    DataFinder m_datafinder;
    QFutureWatcher<void> m_loading_watcher;

    // used to trigger re-rendering of texts on locale change
    QString emptyString() const { return QString(); }
};
