#pragma once

#include <QString>

class QXmlStreamReader;
namespace Model { class Game; }
namespace Model { class Platform; }


namespace Es2 {

/// Compatibility class for parsing ES2 gamelists
class Gamelist {
public:
    static void read(const Model::Platform&);

private:
    static QString findGamelistFile(const Model::Platform&);
    static void parseGamelistFile(QXmlStreamReader&, const Model::Platform&);

    static QHash<QString, QString> readGameProperties();
    static void parseGameTag(QXmlStreamReader&,
                             const Model::Platform&,
                             QHash<QString, Model::Game*>&);
};

} // namespace Es2
