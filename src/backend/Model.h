#pragma once

#include <QString>
#include <QQmlListProperty>


// NOTE: `QQmlListProperty` requires including the namespace in
// the template parameter! See QTBUG-15459.

namespace Model {

class GameAssets : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString boxFront MEMBER m_box_front CONSTANT)
    Q_PROPERTY(QString boxBack MEMBER m_box_back CONSTANT)
    Q_PROPERTY(QString boxSpine MEMBER m_box_spine CONSTANT)
    Q_PROPERTY(QString box MEMBER m_box_full CONSTANT)
    Q_PROPERTY(QString cartridge MEMBER m_cartridge CONSTANT)
    Q_PROPERTY(QString logo MEMBER m_logo CONSTANT)
    Q_PROPERTY(QString marquee MEMBER m_marquee CONSTANT)
    Q_PROPERTY(QString bezel MEMBER m_bezel CONSTANT)
    Q_PROPERTY(QString gridicon MEMBER m_gridicon CONSTANT)
    Q_PROPERTY(QString flyers MEMBER m_flyer CONSTANT)

    // TODO: these could be optimized, see
    // https://doc.qt.io/qt-5/qtqml-cppintegration-data.html (Sequence Type to JavaScript Array)
    Q_PROPERTY(QStringList fanarts MEMBER m_fanarts CONSTANT)
    Q_PROPERTY(QStringList screenshots MEMBER m_screenshots CONSTANT)
    Q_PROPERTY(QStringList videos MEMBER m_videos CONSTANT)

public:
    explicit GameAssets(QObject* parent = nullptr);

    QString m_box_front;
    QString m_box_back;
    QString m_box_spine;
    QString m_box_full;
    QString m_cartridge;
    QString m_logo;
    QString m_marquee;
    QString m_bezel;
    QString m_gridicon;
    QString m_flyer;
    QStringList m_fanarts;
    QStringList m_screenshots;
    QStringList m_videos;
};


class Game : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title MEMBER m_title CONSTANT)
    Q_PROPERTY(QString description MEMBER m_description CONSTANT)
    Q_PROPERTY(QString developer MEMBER m_developer CONSTANT)
    Q_PROPERTY(QString publisher MEMBER m_publisher CONSTANT)
    Q_PROPERTY(QString genre MEMBER m_genre CONSTANT)
    Q_PROPERTY(Model::GameAssets* assets MEMBER m_assets CONSTANT)

public:
    explicit Game(QObject* parent = nullptr);

    QString m_rom_path;
    QString m_rom_basename;

    QString m_title;
    QString m_description;
    QString m_developer;
    QString m_publisher;
    QString m_genre;

    GameAssets* m_assets;
};


class Platform : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString shortName MEMBER m_short_name CONSTANT)
    Q_PROPERTY(QString longName MEMBER m_long_name CONSTANT)
    Q_PROPERTY(QQmlListProperty<Model::Game> games READ getGamesProp CONSTANT)

public:
    explicit Platform(QObject* parent = nullptr);

    QQmlListProperty<Model::Game> getGamesProp();

    QString m_short_name;
    QString m_long_name;
    QString m_rom_dir_path;
    QString m_launch_cmd;

    QList<Game*> m_games;
};

} // namespace Model
