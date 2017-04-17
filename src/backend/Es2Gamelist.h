#pragma once

#include <QFile>
#include <QString>
#include <QXmlStreamReader>

namespace Model { class Game; }
namespace Model { class Platform; }


namespace Es2 {

/// Compatibility class for parsing ES2 gamelists
class Gamelist {
public:
    static void read(const Model::Platform*);

private:
    static void readGamelistFile(const Model::Platform*);

    static QString findGamelistFile(const Model::Platform*);
    static void openGamelistFile(QFile&);
    static void parseGamelistFile(const Model::Platform*);

    static QHash<QString, QString> readGameProperties();
    static void handleGameTag(const Model::Platform*,
                              QHash<QString, Model::Game*>&);

    static QString xml_path;
    static QXmlStreamReader xml;
};

} // namespace Es2
