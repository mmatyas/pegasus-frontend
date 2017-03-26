#include "Model.h"


namespace Model {

GameItem::GameItem(QObject* parent)
    : QObject(parent)
{}

GameModel::GameModel(QObject* parent)
    : QAbstractListModel(parent)
{}

void GameModel::append(GameItemPtr game)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    games << game;
    endInsertRows();
}

int GameModel::rowCount(const QModelIndex&) const {
    return games.count();
}

QVariant GameModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= games.count()) {
        Q_ASSERT(false);
        return QVariant();
    }

    const GameItemPtr& game = games.at(index.row());
    switch (role) {
        case Roles::TitleRole:
            return game->title;
        default:
            break;
    }

    Q_ASSERT(false);
    return QVariant();
}

QHash<int, QByteArray> GameModel::roleNames() const {
    static const QHash<int, QByteArray> roles = {
        { Roles::TitleRole, "title" },
    };

    return roles;
}

PlatformItem::PlatformItem(QObject* parent)
    : QObject(parent)
{}

PlatformModel::PlatformModel(QObject* parent)
    : QAbstractListModel(parent)
{}

void PlatformModel::append(PlatformItemPtr platform)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    platforms << platform;
    endInsertRows();
}

int PlatformModel::rowCount(const QModelIndex&) const {
    return platforms.count();
}

QVariant PlatformModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= platforms.count()) {
        Q_ASSERT(false);
        return QVariant();
    }

    const PlatformItemPtr& platform = platforms.at(index.row());
    switch (role) {
        case Roles::ShortNameRole:
            return platform->short_name;
        case Roles::LongNameRole:
            return platform->long_name;
        case Roles::GameModelRole:
            return QVariant::fromValue<GameModel*>(&platform->game_model);
        case Roles::GameCountRole:
            return platform->game_model.rowCount();
        default:
            break;
    }

    Q_ASSERT(false);
    return QVariant();
}

QHash<int, QByteArray> PlatformModel::roleNames() const {
    static const QHash<int, QByteArray> roles = {
        { Roles::ShortNameRole, "shortName" },
        { Roles::LongNameRole, "longName" },
        { Roles::GameModelRole, "gameModel" },
        { Roles::GameCountRole, "gameCount" },
    };

    return roles;
}

} // namespace Model
