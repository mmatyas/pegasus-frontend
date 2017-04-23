#pragma once

#include <QList>

namespace Model { class Game; }
namespace Model { class Platform; }


// TODO: make this a QObject with progress signals
class DataFinder {
public:
    static void find(QList<Model::Platform*>&);

private:
    static void findPlatforms(QList<Model::Platform*>&);
    static void findPlatformGames(Model::Platform*);
    static void removeEmptyPlatforms(QList<Model::Platform*>&);

    static void findGameMetadata(const Model::Platform&);
    static void findGameAssets(const Model::Platform&);
};
