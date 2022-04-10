// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include "utils/HashMap.h"
#include "utils/NoCopyNoMove.h"

#include <QObject>
#include <QStringList>
#include <vector>

namespace model { class Game; }
namespace model { class GameFile; }
namespace model { class Collection; }
class QNetworkAccessManager;
class QNetworkReply;
class QUrl;


namespace providers {

class SearchContext : public QObject {
    Q_OBJECT

public:
    explicit SearchContext(QObject* parent = nullptr);
    explicit SearchContext(QStringList, QObject* parent = nullptr);
    NO_COPY_NO_MOVE(SearchContext)

    model::Collection* get_or_create_collection(const QString&);
    model::Game* create_game_for(model::Collection&);
    model::Game* create_game();
    SearchContext& game_add_to(model::Game&, model::Collection&);

    model::Game* game_by_filepath(const QString&) const;
    model::Game* game_by_uri(const QString&) const;
    model::GameFile* gamefile_by_filepath(const QString&) const;
    model::GameFile* gamefile_by_uri(const QString&) const;
    model::GameFile* game_add_filepath(model::Game&, QString);
    model::GameFile* game_add_uri(model::Game&, QString);

    const QStringList& root_game_dirs() const { return m_root_game_dirs; }
    const QStringList& pegasus_game_dirs() const { return m_pegasus_game_dirs; }
    SearchContext& pegasus_add_game_dir(QString);

    SearchContext& enable_network();
    bool has_network() const;
    SearchContext& schedule_download(const QUrl&, const std::function<void(QNetworkReply* const)>&);
    bool has_pending_downloads() const;

    const HashMap<QString, model::GameFile*>& current_filepath_to_entry_map() const { return m_filepath_to_gamefile; }
    std::pair<std::vector<model::Collection*>, std::vector<model::Game*>> finalize(QObject* const parent = nullptr);

signals:
    void downloadScheduled();
    void downloadCompleted();

private:
    const QStringList m_root_game_dirs;
    QStringList m_pegasus_game_dirs;

    QNetworkAccessManager* m_netman;
    std::atomic<size_t> m_pending_downloads;

    HashMap<QString, model::Collection*> m_collections;
    HashMap<model::Collection*, std::vector<model::Game*>> m_collection_games;
    HashMap<model::Game*, std::vector<model::GameFile*>> m_game_entries;
    HashMap<QString, model::GameFile*> m_filepath_to_gamefile;
    HashMap<QString, model::GameFile*> m_uri_to_gamefile;

    std::vector<model::Game*> m_parentless_games;

    void finalize_cleanup_games();
    void finalize_cleanup_collections();
    void finalize_apply_lists();
};
} // namespace providers
