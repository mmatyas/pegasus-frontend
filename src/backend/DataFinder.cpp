#include "DataFinder.h"

#include "Es2AssetFinder.h"
#include "Es2Gamelist.h"
#include "Es2Systems.h"
#include "Model.h"

#include <QDirIterator>


void DataFinder::find(QList<Model::Platform*>& model)
{
    Q_ASSERT(model.isEmpty());

    findPlatforms(model);

    for (Model::Platform* platform : model)
        findPlatformGames(platform);
    removeEmptyPlatforms(model);

    for (const Model::Platform* platform : model) {
        findGameMetadata(*platform);
        findGameAssets(*platform);
    }
}

void DataFinder::findPlatforms(QList<Model::Platform*>& model)
{
    Q_ASSERT(model.isEmpty());

    // at the moment, we only use ES2's platform definitions
    const QVector<Model::Platform*> es2_platforms = Es2::Systems::read();
    for (auto& platform : es2_platforms)
        model.append(platform);
}

void DataFinder::findPlatformGames(Model::Platform* platform)
{
    static const auto filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    static const auto flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    Q_ASSERT(platform);
    Q_ASSERT(!platform->m_rom_dir_path.isEmpty());
    Q_ASSERT(!platform->m_rom_filters.isEmpty()); // FIXME: handle incorrect filters

    QDirIterator romdir_it(platform->m_rom_dir_path,
                           platform->m_rom_filters,
                           filters, flags);
    while (romdir_it.hasNext())
        platform->m_games.append(new Model::Game(romdir_it.next(), platform));
}

void DataFinder::removeEmptyPlatforms(QList<Model::Platform*>& platforms)
{
    // NOTE: if this turns out to be slow, STL iterators
    // could be used here
    QMutableListIterator<Model::Platform*> it(platforms);
    while (it.hasNext()) {
        if (it.next()->m_games.isEmpty())
            it.remove();
    }
}

void DataFinder::findGameMetadata(const Model::Platform& platform)
{
    Es2::Gamelist::read(platform);
}

void DataFinder::findGameAssets(const Model::Platform& platform)
{
    for (Model::Game* game : platform.m_games) {
        Q_ASSERT(game);
        Es2::AssetFinder::findAll(platform, *game);
    }
}
