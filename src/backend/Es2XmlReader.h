#pragma once

#include "Model.h"

#include <QXmlStreamReader>


class Es2XmlReader {
public:
    static bool read(QList<Model::Platform*>&);

private:
    static QVector<Model::Platform*> readSystemsFile();
    static QString findSystemsCfg();
    static Model::Platform* readSystem();
    static void parseSystemShortName(Model::Platform*);
    static void parseSystemRomDirPath(Model::Platform*);
    static void parseSystemRunCmd(Model::Platform*);

    static QVector<Model::Game*> readGamelistFile(Model::Platform*);
    static QString findGamelist(const Model::Platform*);
    static Model::Game* readGame(Model::Platform*);
    static void parseGamePath(Model::Game*);
    static void parseGameName(Model::Game*);
    static void parseGameDescription(Model::Game*);
    static void parseGameDeveloper(Model::Game*);
    static void findGameAssets(Model::Platform*, Model::Game*);

    static QXmlStreamReader xml;
};

class Es2Assets {
public:
    enum class AssetType : uint8_t {
        BOX_FRONT,
        LOGO,
        SCREENSHOT,
        VIDEO,
    };

    static QString find(AssetType, const Model::Platform*, const Model::Game*);

private:
    static QVector<QString> possibleSuffixes(AssetType);
    static QVector<QString> possibleExtensions(AssetType);
};
