#pragma once

#include <QString>
#include <QXmlStreamReader>

namespace Model { class Game; }
namespace Model { class Platform; }


class Es2GamelistReader {
public:
    static QVector<Model::Game*> read(Model::Platform*);

    // TODO: make this smarter
    static void findGameAssets(Model::Platform*, Model::Game*);

private:
    static QString findGamelist(const Model::Platform*);
    static Model::Game* readGame(Model::Platform*);
    static void parseGamePath(Model::Game*);
    static void parseGameName(Model::Game*);
    static void parseGameDescription(Model::Game*);
    static void parseGameDeveloper(Model::Game*);

    static QXmlStreamReader xml;
};
