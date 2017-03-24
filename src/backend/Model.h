#pragma once

#include "PlatformList.h"

#include <QAbstractListModel>
#include <QSharedPointer>
#include <QString>
#include <QVector>


namespace Model {

class GameItem : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString romPath MEMBER rom_path CONSTANT)
    Q_PROPERTY(QString title MEMBER title CONSTANT)
    Q_PROPERTY(QString description MEMBER description CONSTANT)
    Q_PROPERTY(QString developer MEMBER developer CONSTANT)

public:
    explicit GameItem(QObject* parent = nullptr);

    QString rom_path;

    QString title;
    QString description;
    QString developer;
};

using GameItemPtr = QSharedPointer<GameItem>;

class GameModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
    };

    explicit GameModel(QObject* parent = nullptr);

    void append(GameItemPtr&);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QVector<GameItemPtr> games;
};

using GameModelPtr = QSharedPointer<GameModel>;


class PlatformItem : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString shortName MEMBER short_name CONSTANT)
    Q_PROPERTY(QString longName MEMBER long_name CONSTANT)
    Q_PROPERTY(GameModelPtr gameModel MEMBER game_model CONSTANT)

public:
    explicit PlatformItem(QObject* parent = nullptr);

    QString short_name;
    QString long_name;
    QString rom_dir_path;
    QString launch_cmd;

    GameModelPtr game_model;
};

using PlatformItemPtr = QSharedPointer<Model::PlatformItem>;

class PlatformModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        ShortNameRole = Qt::UserRole + 1,
        LongNameRole,
        GameModelRole,
    };

    explicit PlatformModel(QObject* parent = nullptr);

    void append(PlatformItemPtr&);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QVector<PlatformItemPtr> platforms;
};

} // namespace Model
