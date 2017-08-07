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

#include "Assets.h"

#include <QDateTime>
#include <QString>
#include <QQmlListProperty>


// NOTE: `QQmlListProperty` requires including the namespace in
// the template parameter! See QTBUG-15459.

namespace Model {

class GameAssets : public QObject {
    Q_OBJECT

    // NOTE: by manually listing the properties (instead of eg. a Map),
    //       it is also possible to refer the same data by a different name
    Q_PROPERTY(QString boxFront READ boxFront CONSTANT)
    Q_PROPERTY(QString boxBack READ boxBack CONSTANT)
    Q_PROPERTY(QString boxSpine READ boxSpine CONSTANT)
    Q_PROPERTY(QString boxFull READ boxFull CONSTANT)
    Q_PROPERTY(QString box READ boxFull CONSTANT)
    Q_PROPERTY(QString cartridge READ cartridge CONSTANT)
    Q_PROPERTY(QString logo READ logo CONSTANT)
    Q_PROPERTY(QString marquee READ marquee CONSTANT)
    Q_PROPERTY(QString bezel READ bezel CONSTANT)
    Q_PROPERTY(QString gridicon READ gridicon CONSTANT)
    Q_PROPERTY(QString flyer READ flyer CONSTANT)
    Q_PROPERTY(QString music READ music CONSTANT)

    // TODO: these could be optimized, see
    // https://doc.qt.io/qt-5/qtqml-cppintegration-data.html (Sequence Type to JavaScript Array)
    Q_PROPERTY(QStringList fanarts READ fanarts CONSTANT)
    Q_PROPERTY(QStringList screenshots READ screenshots CONSTANT)
    Q_PROPERTY(QStringList videos READ videos CONSTANT)

public:
    explicit GameAssets(QObject* parent = nullptr);

    QHash<Assets::Type, QString> m_single_assets;
    QHash<Assets::Type, QStringList> m_multi_assets;

    void setSingle(Assets::Type, QString);
    void appendMulti(Assets::Type, QString);

    QString boxFront() const { return m_single_assets[Assets::Type::BOX_FRONT]; }
    QString boxBack() const { return m_single_assets[Assets::Type::BOX_BACK]; }
    QString boxSpine() const { return m_single_assets[Assets::Type::BOX_SPINE]; }
    QString boxFull() const { return m_single_assets[Assets::Type::BOX_FULL]; }
    QString cartridge() const { return m_single_assets[Assets::Type::CARTRIDGE]; }
    QString logo() const { return m_single_assets[Assets::Type::LOGO]; }
    QString marquee() const { return m_single_assets[Assets::Type::MARQUEE]; }
    QString bezel() const { return m_single_assets[Assets::Type::BEZEL]; }
    QString gridicon() const { return m_single_assets[Assets::Type::STEAMGRID]; }
    QString flyer() const { return m_single_assets[Assets::Type::FLYER]; }
    QString music() const { return m_single_assets[Assets::Type::MUSIC]; }

    QStringList fanarts() const { return m_multi_assets[Assets::Type::FANARTS]; }
    QStringList screenshots() const { return m_multi_assets[Assets::Type::SCREENSHOTS]; }
    QStringList videos() const { return m_multi_assets[Assets::Type::VIDEOS]; }
};


class Game : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title MEMBER m_title CONSTANT)
    Q_PROPERTY(QString description MEMBER m_description CONSTANT)
    Q_PROPERTY(QString developer MEMBER m_developer CONSTANT)
    Q_PROPERTY(QString publisher MEMBER m_publisher CONSTANT)
    Q_PROPERTY(QString genre MEMBER m_genre CONSTANT)
    Q_PROPERTY(int year MEMBER m_year CONSTANT)
    Q_PROPERTY(int month MEMBER m_month CONSTANT)
    Q_PROPERTY(int day MEMBER m_day CONSTANT)
    Q_PROPERTY(int players MEMBER m_players CONSTANT)
    Q_PROPERTY(int playCount MEMBER m_playcount CONSTANT)
    Q_PROPERTY(float rating MEMBER m_rating CONSTANT)
    Q_PROPERTY(QDateTime lastPlayed MEMBER m_lastplayed CONSTANT)
    Q_PROPERTY(Model::GameAssets* assets READ assets CONSTANT)

public:
    explicit Game(QString path, QObject* parent = nullptr);

    const QString m_rom_path;
    const QString m_rom_basename;

    QString m_title;
    QString m_description;
    QString m_developer;
    QString m_publisher;
    QString m_genre;

    int m_players;
    float m_rating;

    int m_year;
    int m_month;
    int m_day;

    int m_playcount;
    QDateTime m_lastplayed;

    GameAssets* assets() const { return m_assets; }

private:
    GameAssets* m_assets;
};


class Platform : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString shortName MEMBER m_short_name CONSTANT)
    Q_PROPERTY(QString longName MEMBER m_long_name CONSTANT)

    Q_PROPERTY(int currentGameIndex
               READ currentGameIndex WRITE setCurrentGameIndex
               RESET resetGameIndex
               NOTIFY currentGameIndexChanged)
    Q_PROPERTY(Model::Game* currentGame
               READ currentGame
               NOTIFY currentGameChanged)
    Q_PROPERTY(QQmlListProperty<Model::Game> games READ getGamesProp CONSTANT)

public:
    explicit Platform(QString name, QString rom_dir_path,
                      QStringList rom_filters, QString launch_cmd,
                      QObject* parent = nullptr);

    int currentGameIndex() const { return m_current_game_idx; }
    void setCurrentGameIndex(int);
    void resetGameIndex();

    Model::Game* currentGame() const { return m_current_game; }

    QQmlListProperty<Model::Game> getGamesProp();

    const QString m_short_name;
    const QString m_long_name;
    const QString m_rom_dir_path;
    const QStringList m_rom_filters;
    const QString m_launch_cmd;

    QList<Game*> m_games;

signals:
    void currentGameIndexChanged();
    void currentGameChanged();

private:
    int m_current_game_idx;
    Model::Game* m_current_game;
};

} // namespace Model
