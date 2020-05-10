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


#include <QtQuickTest>

#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"

#include "SortFilterProxyModel/qqmlsortfilterproxymodel.h"
#include "SortFilterProxyModel/filters/filtersqmltypes.h"
#include "SortFilterProxyModel/proxyroles/proxyrolesqmltypes.h"
#include "SortFilterProxyModel/sorters/sortersqmltypes.h"
#include "QtQmlTricks/QQmlObjectListModel.h"
#include <QQmlEngine>
#include <QQmlContext>


class Setup : public QObject {
    Q_OBJECT

public:
    Setup()
    {
        register_api();
        create_model();
    }

public slots:
    void qmlEngineAvailable(QQmlEngine* engine)
    {
        engine->rootContext()->setContextProperty("collections", &m_collection_model);
    }

private:
    QQmlObjectListModel<model::Collection> m_collection_model;

    void register_api() {
        constexpr auto api = "Pegasus.Test";
        const auto err = QStringLiteral("Error!");

        qmlRegisterUncreatableType<model::Collection>(api, 1, 0, "Collection", err);
        qmlRegisterUncreatableType<model::Game>(api, 1, 0, "Game", err);
        qmlRegisterUncreatableType<model::Assets>(api, 1, 0, "Assets", err);

        qqsfpm::registerSorterTypes();
        qqsfpm::registerFiltersTypes();
        qqsfpm::registerProxyRoleTypes();
        qqsfpm::registerQQmlSortFilterProxyModelTypes();
    }

    void create_model() {
        auto collection = new model::Collection("test", this);
        (*collection)
            .addGame(new model::Game(QFileInfo("ccc"), this))
            .addGame(new model::Game(QFileInfo("aaa"), this))
            .addGame(new model::Game(QFileInfo("bbb"), this))
            .finalize();
        m_collection_model.append(collection);
    }
};


QUICK_TEST_MAIN_WITH_SETUP(SortFilter, Setup)
#include "test_SortFilter.moc"
