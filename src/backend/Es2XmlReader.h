#pragma once

#include <QXmlStreamReader>

namespace Model { class Platform; }


class Es2XmlReader {
public:
    static bool read();

private:
    static QString es2SystemsCfgPath();

    static void parseSystem();
    static void parseSystemName(Model::Platform&);
    static void parseSystemRomDirPath(Model::Platform&);
    static void parseSystemRunCmd(Model::Platform&);

    static QXmlStreamReader xml;
};
