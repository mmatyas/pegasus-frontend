#pragma once

#include "Model.h"

#include <QXmlStreamReader>


class Es2XmlReader {
public:
    static bool read(Model::PlatformModel&);

private:
    static QVector<Model::PlatformItemPtr> readSystemsFile();
    static QString findSystemsCfg();
    static Model::PlatformItemPtr readSystem();
    static void parseSystemShortName(Model::PlatformItemPtr&);
    static void parseSystemRomDirPath(Model::PlatformItemPtr&);
    static void parseSystemRunCmd(Model::PlatformItemPtr&);

    static QVector<Model::GameItemPtr> readGamelistFile(const Model::PlatformItemPtr&);
    static QString findGamelist(const Model::PlatformItemPtr&);
    static Model::GameItemPtr readGame();
    static void parseGamePath(Model::GameItemPtr&);
    static void parseGameName(Model::GameItemPtr&);
    static void parseGameDescription(Model::GameItemPtr&);
    static void parseGameDeveloper(Model::GameItemPtr&);
    static void findGameAssets(Model::PlatformItemPtr&, Model::GameItemPtr&);

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

    static QString find(AssetType, const Model::PlatformItemPtr&, const Model::GameItemPtr&);

private:
    static QVector<QString> possibleSuffixes(AssetType);
    static QVector<QString> possibleExtensions(AssetType);
};
