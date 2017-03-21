#pragma once

#include <QXmlStreamReader>

namespace Model { class Platform; }


class Es2XmlReader {
public:
    static bool read();

private:
    static QString systemsCfgPath();
    static void readSystem();
    static void parseSystemShortName(Model::Platform&);
    static void parseSystemRomDirPath(Model::Platform&);
    static void parseSystemRunCmd(Model::Platform&);

    static QXmlStreamReader xml;
};
