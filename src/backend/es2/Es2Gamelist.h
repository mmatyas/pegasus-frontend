// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


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
