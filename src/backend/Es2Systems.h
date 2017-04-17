#pragma once

#include <QFile>
#include <QXmlStreamReader>

namespace Model { class Platform; }


namespace Es2 {

/// Compatibility class for parsing ES2 game system definitions
class Systems {
public:
    static QVector<Model::Platform*> read();

private:
    static QVector<Model::Platform*> readSystemsFile();

    static QString findSystemsFile();
    static void openSystemsFile(QFile&);
    static QVector<Model::Platform*> parseSystemsFile();

    static Model::Platform* readSystemTag();
    static void parseSystemShortName(Model::Platform*);
    static void parseSystemRomDirPath(Model::Platform*);
    static void parseSystemRunCmd(Model::Platform*);
    static void parseSystemExtensions(Model::Platform*);

    static QString xml_path;
    static QXmlStreamReader xml;
};

} // namespace Es2
