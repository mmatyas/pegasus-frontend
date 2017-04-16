#pragma once

#include <QXmlStreamReader>

namespace Model { class Platform; }


namespace Es2 {

/// Compatibility class for parsing ES2 game system definitions
class Systems {
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

} // namespace Es2
