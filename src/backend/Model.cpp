#include "Model.h"


namespace Model {

PlatformModel::PlatformModel(QObject* parent)
    : QAbstractListModel(parent)
{}

void PlatformModel::append(const Platform& platform)
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

    const Platform& platform = platforms.at(index.row());
    switch (role) {
        case PlatformRoles::ShortNameRole:
            return platform.short_name;
        case PlatformRoles::LongNameRole:
            return platform.long_name;
        default:
            break;
    }

    Q_ASSERT(false);
    return QVariant();
}

QHash<int, QByteArray> PlatformModel::roleNames() const {
    static const QHash<int, QByteArray> roles = {
        { PlatformRoles::ShortNameRole, "shortName" },
        { PlatformRoles::LongNameRole, "longName" },
    };

    return roles;
}

} // namespace Model
