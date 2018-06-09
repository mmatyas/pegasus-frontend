#pragma once

#include "Assets.h"
#include "utils/HashMap.h"

#include <QString>
#include <QStringList>


namespace modeldata {

struct GameAssets {
    explicit GameAssets();

    // NOTE: a value for an asset type may not exist at the time of calling
    const QString& single(AssetType type) { return m_single_assets[type]; }
    const QStringList& multi(AssetType type) { return m_multi_assets[type]; }

    void setSingle(AssetType, QString);
    void appendMulti(AssetType, QString);


    GameAssets(const GameAssets&) = delete;
    GameAssets& operator=(const GameAssets&) = delete;
    GameAssets(GameAssets&&) = default;
    GameAssets& operator=(GameAssets&&) = default;

private:
    HashMap<AssetType, QString> m_single_assets;
    HashMap<AssetType, QStringList> m_multi_assets;
};

} // namespace modeldata
