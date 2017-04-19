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
    static QString findSystemsFile();
    static QVector<Model::Platform*> parseSystemsFile(QFile&);

    static Model::Platform* parseSystemTag();
    static void processRomDir(QString&);
    static QStringList parseFilters(const QString&);

    static QXmlStreamReader xml;
};

} // namespace Es2
