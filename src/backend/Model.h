#pragma once

#include "PlatformList.h"

#include <QAbstractListModel>
#include <QString>
#include <QVector>


namespace Model {

struct GameAssets {
    QString box_front_path;
};

struct Game {
    QString path;

    QString title;
    QString description;
    QString developer;

    GameAssets assets;
};

struct Platform {
    QString short_name;
    QString long_name;
    QString rom_dir_path;
    QString launch_cmd;

    QVector<Game> games;
};

class PlatformModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum PlatformRoles {
        ShortNameRole = Qt::UserRole + 1,
        LongNameRole,
    };

    explicit PlatformModel(QObject* parent = nullptr);

    void append(const Platform&);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QVector<Platform> platforms;
};

} // namespace Model
