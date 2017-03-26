#pragma once

#include "PlatformList.h"

#include <QAbstractListModel>
#include <QSharedPointer>
#include <QString>
#include <QVector>


namespace Model {

class GameAssets : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString boxFront MEMBER box_front CONSTANT)
    Q_PROPERTY(QString logo MEMBER logo CONSTANT)

public:
    explicit GameAssets(QObject* parent = nullptr);

    Q_INVOKABLE const QStringList& screenshots() const {
        return screenshot_list;
    }
    Q_INVOKABLE const QStringList& videos() const {
        return video_list;
    }

    QString box_front;
    QString logo;
    QStringList screenshot_list;
    QStringList video_list;
};


class GameItem : public QObject {
    Q_OBJECT

public:
    explicit GameItem(QObject* parent = nullptr);

    QString rom_path;

    QString title;
    QString description;
    QString developer;
    QString publisher;
    QString genre;

    GameAssets assets;
};

using GameItemPtr = GameItem*;

class GameModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        DescriptionRole,
        DeveloperRole,
        PublisherRole,
        GenreRole,
        AssetsRole,
    };

    explicit GameModel(QObject* parent = nullptr);

    void append(GameItemPtr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QVector<GameItemPtr> games;
};


class PlatformItem : public QObject {
    Q_OBJECT

public:
    explicit PlatformItem(QObject* parent = nullptr);

    QString short_name;
    QString long_name;
    QString rom_dir_path;
    QString launch_cmd;

    GameModel game_model;
};

using PlatformItemPtr = PlatformItem*;

class PlatformModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        ShortNameRole = Qt::UserRole + 1,
        LongNameRole,
        GameModelRole,
        GameCountRole,
    };

    explicit PlatformModel(QObject* parent = nullptr);

    void append(PlatformItemPtr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QVector<PlatformItemPtr> platforms;
};

} // namespace Model

Q_DECLARE_METATYPE(Model::GameAssets*)
