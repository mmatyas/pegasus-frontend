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

#include "GameAssets.h"
#include "modeldata/gaming/CollectionData.h"
#include "modeldata/gaming/GameData.h"

#include <QObject>


namespace {
QString joined_list(const QStringList& list) { return list.join(QLatin1String(", ")); }
} // namespace


#define CPROP_Q(type, apiName) \
    private: Q_PROPERTY(type apiName READ apiName CONSTANT)

#define CPROP_REF(type, apiName, dataField) \
    public: const type& apiName() const { return m_game.dataField; } \
    CPROP_Q(type, apiName)

#define CPROP_POD(type, apiName, dataField) \
    public: type apiName() const { return m_game.dataField; } \
    CPROP_Q(type, apiName)


namespace model {
class Game : public QObject {
    Q_OBJECT

    CPROP_REF(QString, title, title)
    CPROP_REF(QString, summary, summary)
    CPROP_REF(QString, description, description)

    Q_PROPERTY(QString developer READ developerString CONSTANT)
    Q_PROPERTY(QString publisher READ publisherString CONSTANT)
    Q_PROPERTY(QString genre READ genreString CONSTANT)
    CPROP_REF(QStringList, developerList, developers)
    CPROP_REF(QStringList, publisherList, publishers)
    CPROP_REF(QStringList, genreList, genres)

    CPROP_POD(int, players, player_count)
    CPROP_POD(float, rating, rating)

    CPROP_REF(QDate, release, release_date)
    CPROP_POD(int, releaseYear, release_date.year())
    CPROP_POD(int, releaseMonth, release_date.month())
    CPROP_POD(int, releaseDay, release_date.day())

    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    Q_PROPERTY(int playCount READ playCount NOTIFY playStatsChanged)
    Q_PROPERTY(int playTime READ playTime NOTIFY playStatsChanged)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed NOTIFY playStatsChanged)

    Q_PROPERTY(model::GameAssets* assets READ assetsPtr CONSTANT)

public:
    explicit Game(modeldata::Game, QObject* parent = nullptr);

    Q_INVOKABLE void launch();

    const modeldata::Game& data() const { return m_game; }
    void setFavorite(bool);
    void addPlayStats(int playcount, qint64 playtime, const QDateTime& last_played);
    void updatePlayStats(qint64 duration, QDateTime time_finished);

signals:
    void launchRequested(model::Game*);

    void favoriteChanged();
    void playStatsChanged();

private:
    QString developerString() const { return joined_list(m_game.developers); }
    QString publisherString() const { return joined_list(m_game.publishers); }
    QString genreString() const { return joined_list(m_game.genres); }

    bool favorite() const { return m_game.is_favorite; }
    int playCount() const { return m_game.playcount; }
    qint64 playTime() const { return m_game.playtime; }
    const QDateTime& lastPlayed() const { return m_game.last_played; }

    GameAssets* assetsPtr() { return &m_assets; }

private:
    modeldata::Game m_game;
    GameAssets m_assets;
};
} // namespace model


#undef CPROP_REF
#undef CPROP_POD
#undef CPROP_Q
