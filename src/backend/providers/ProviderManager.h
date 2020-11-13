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
    explicit ProviderManager(QObject* parent);

    void run(QVector<model::Collection*>&, QVector<model::Game*>&);

    void onGameLaunched(model::GameFile* const) const;
    void onGameFinished(model::GameFile* const) const;
    void onGameFavoriteChanged(const QVector<model::Game*>&) const;

signals:
    void progressChanged(float); // TODO: add displayed text
    void finished();

private slots:
    void onProviderProgressChanged(float);

private:
    QFuture<void> m_future;
    float m_progress_finished;
    float m_progress_provider_weight;

    QVector<model::Collection*>* m_target_collection_list;
    QVector<model::Game*>* m_target_game_list;

    void finalize();
};
