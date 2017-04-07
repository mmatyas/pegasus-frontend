#pragma once

#include "Model.h"

#include <QXmlStreamReader>


class Es2XmlReader {
public:
    static bool read(Model::PlatformModel&);

private:
    static QVector<Model::PlatformItemPtr> readSystemsFile();
    static QString systemsCfgPath();
    static Model::PlatformItemPtr readSystem();
    static void parseSystemShortName(Model::PlatformItemPtr&);
    static void parseSystemRomDirPath(Model::PlatformItemPtr&);
    static void parseSystemRunCmd(Model::PlatformItemPtr&);

    static QVector<Model::GameItemPtr> readGamelistFile(const Model::PlatformItemPtr&);
    static QString gamelistPath(const Model::PlatformItemPtr&);
    static Model::GameItemPtr readGame();
    static void parseGamePath(Model::GameItemPtr&);
    static void parseGameName(Model::GameItemPtr&);
    static void parseGameDescription(Model::GameItemPtr&);
    static void parseGameDeveloper(Model::GameItemPtr&);

    static QString gameAssetPath(const Model::PlatformItemPtr&,
                                 const Model::GameItemPtr&, const QString&);
    static void findGameAssets(Model::PlatformItemPtr&, Model::GameItemPtr&);

    static QXmlStreamReader xml;
};
