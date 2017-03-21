#pragma once

#include <QXmlStreamReader>

namespace Model { class Game; }
namespace Model { class Platform; }


class Es2XmlReader {
public:
    static bool read();

private:
    static QVector<Model::Platform> readSystemsFile();
    static QString systemsCfgPath();
    static Model::Platform readSystem();
    static void parseSystemShortName(Model::Platform&);
    static void parseSystemRomDirPath(Model::Platform&);
    static void parseSystemRunCmd(Model::Platform&);

    static QVector<Model::Game> readGamelistFile(const Model::Platform&);
    static QString gamelistPath(const QString&);
    static Model::Game readGame();
    static void parseGamePath(Model::Game&);
    static void parseGameName(Model::Game&);
    static void parseGameDescription(Model::Game&);
    static void parseGameDeveloper(Model::Game&);
    static void parseGameImage(Model::Game&);

    static QXmlStreamReader xml;
};
