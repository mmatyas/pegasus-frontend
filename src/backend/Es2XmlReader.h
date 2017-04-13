#pragma once

#include <QXmlStreamReader>

namespace Model { class Platform; }


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

    static QXmlStreamReader xml;
};
