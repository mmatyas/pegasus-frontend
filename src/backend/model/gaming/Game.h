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

#include "GameAssets.h"
#include "GameFile.h"
#include "modeldata/CollectionData.h"
#include "modeldata/GameData.h"

#include "QtQmlTricks/QQmlObjectListModel.h"
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
    CPROP_REF(QString, sortTitle, sort_title)
    CPROP_REF(QString, summary, summary)
    CPROP_REF(QString, description, description)

    Q_PROPERTY(QString developer READ developerString CONSTANT)
    Q_PROPERTY(QString publisher READ publisherString CONSTANT)
    Q_PROPERTY(QString genre READ genreString CONSTANT)
    CPROP_REF(QStringList, developerList, developers)
    CPROP_REF(QStringList, publisherList, publishers)
    CPROP_REF(QStringList, genreList, genres)
    CPROP_REF(QStringList, tagList, tags)

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
    QML_OBJMODEL_PROPERTY(model::GameFile, files)

public:
    explicit Game(modeldata::Game, QObject* parent = nullptr);

    Q_INVOKABLE void launch();

    const modeldata::Game& data() const { return m_game; }
    void setFavorite(bool);

public:
    // a workaround for const pointer issues with the model
    const QVector<model::GameFile*>& filesConst() const { return m_files.asList(); }

    QString developerString() const { return joined_list(m_game.developers); }
    QString publisherString() const { return joined_list(m_game.publishers); }
    QString genreString() const { return joined_list(m_game.genres); }

    bool favorite() const { return m_game.is_favorite; }
    int playCount() const;
    qint64 playTime() const;
    QDateTime lastPlayed() const;

    GameAssets* assetsPtr() { return &m_assets; }

signals:
    void launchFileSelectorRequested();
    void favoriteChanged();
    void playStatsChanged();

private:
    modeldata::Game m_game;
    GameAssets m_assets;
};
} // namespace model


#undef CPROP_REF
#undef CPROP_POD
#undef CPROP_Q
