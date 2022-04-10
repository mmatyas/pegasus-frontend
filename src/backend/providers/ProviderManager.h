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

#include <QObject>
#include <QFuture>

namespace model { class Collection; }
namespace model { class Game; }
namespace model { class GameFile; }


class ProviderManager : public QObject {
    Q_OBJECT

public:
    explicit ProviderManager(QObject* parent = nullptr);

    void run();

    void onGameLaunched(model::GameFile* const) const;
    void onGameFinished(model::GameFile* const) const;
    void onFavoritesChanged(const std::vector<model::Game*>&) const;

    std::vector<model::Collection*>& foundCollections() { return m_found_collections; }
    std::vector<model::Game*>& foundGames() { return m_found_games; }

signals:
    void scanStarted();
    void scanProgressChanged(float, QString);
    void scanFinished();

private slots:
    void onProviderProgressChanged(float);

private:
    QFuture<void> m_future;
    float m_progress_step = 1.f;
    float m_current_progress = 0.f;
    QString m_current_stage;

    std::vector<model::Collection*> m_found_collections;
    std::vector<model::Game*> m_found_games;

    void finalize();
};
