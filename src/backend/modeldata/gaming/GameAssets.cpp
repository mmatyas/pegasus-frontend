#include "GameAssets.h"


namespace modeldata {

GameAssets::GameAssets() = default;

void GameAssets::setSingle(AssetType key, QString value)
{
    Q_ASSERT(Assets::singleTypes.contains(key));
    m_single_assets[key] = std::move(value);
}

void GameAssets::appendMulti(AssetType key, QString value)
{
    Q_ASSERT(Assets::multiTypes.contains(key));
    m_multi_assets[key].append(std::move(value));
}

} // namespace modeldata
