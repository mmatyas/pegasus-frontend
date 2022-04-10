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

#include "CliArgs.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "model/device/DeviceInfo.h"
#include "model/keys/Keys.h"
#include "model/memory/Memory.h"
#include "providers/ProviderManager.h"
#include "utils/QmlHelpers.h"

#include <QObject>


/// Provides data access for QML
///
/// Provides an API for the frontend layer, to allow accessing every public
/// property of the backend from QML.
namespace model {
class ApiObject : public QObject {
    Q_OBJECT

    QML_CONST_PROPERTY(model::DeviceInfo, device)
    QML_CONST_PROPERTY(model::Keys, keys)
    QML_READONLY_PROPERTY(model::Memory, memory)
    Q_PROPERTY(CollectionListModel* collections READ collections CONSTANT)
    Q_PROPERTY(GameListModel* allGames READ allGames CONSTANT)

    // retranslate on locale change
    Q_PROPERTY(QString tr READ emptyString NOTIFY retranslationRequested)

public:
    explicit ApiObject(const backend::CliArgs& args, QObject* parent = nullptr);

    // scanning
    void clearGameData();
    void setGameData(std::vector<model::Collection*>&&, std::vector<model::Game*>&&);

    CollectionListModel* collections() const { return m_collections; }
    GameListModel* allGames() const { return m_all_games; }

signals:
    // loading
    void gamedataReady();

    // user actions
    void launchGameFile(const model::GameFile*);
    void launchFailed(QString);
    void gameFileFinished(model::GameFile* const);
    void gameFileLaunched(model::GameFile* const);
    void favoritesChanged();
    void memoryChanged();

    // triggers translation update
    void retranslationRequested();

    // Api events for QML -- no const here
    void eventSelectGameFile(model::Game* game);
    void eventLaunchError(QString msg);

public slots:
    // game launch communication
    void onGameLaunchOk();
    void onGameLaunchError(QString);
    void onGameProcessFinished();

    // setting changes
    void onLocaleChanged();
    void onThemeChanged(QString);

private slots:
    // internal communication
    void onGameFavoriteChanged();
    void onGameFileSelectorRequested();
    void onGameFileLaunchRequested();

private:
    // game launching
    model::GameFile* m_launch_game_file;

    // used to trigger re-rendering of texts on locale change
    QString emptyString() const { return QString(); }

    CollectionListModel* m_collections = nullptr;
    GameListModel* m_all_games = nullptr;
};
} // namespace model
