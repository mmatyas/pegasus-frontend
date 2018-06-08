#include "GameAssets.h"


namespace modeldata {

GameAssets::GameAssets() = default;

void GameAssets::setSingle(AssetType key, QString value)
{
    Q_ASSERT(asset_is_single(key));
    m_single_assets[key] = std::move(value);
}

void GameAssets::appendMulti(AssetType key, QString value)
{
    Q_ASSERT(!asset_is_single(key));
    m_multi_assets[key].append(std::move(value));
}

} // namespace modeldata
