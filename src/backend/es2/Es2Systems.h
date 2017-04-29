#pragma once

#include <QString>

class QXmlStreamReader;
namespace Model { class Platform; }


namespace Es2 {

/// Compatibility class for parsing ES2 game system definitions
class Systems {
public:
    static QVector<Model::Platform*> read();

private:
    static QString findSystemsFile();
    static QVector<Model::Platform*> parseSystemsFile(QXmlStreamReader&);

    static Model::Platform* parseSystemTag(QXmlStreamReader&);
    static void processRomDir(QString&);
    static QStringList parseFilters(const QString&);
};

} // namespace Es2
