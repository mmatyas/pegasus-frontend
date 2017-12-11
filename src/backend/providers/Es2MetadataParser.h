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

#include <QHash>
#include <QXmlStreamReader>

namespace Types { class Collection; }
namespace Types { class Game; }


namespace providers {
namespace es2_utils {

void parseGamelistFile(QXmlStreamReader&,
                       const Types::Collection&,
                       const QHash<QString, Types::Game*>&);
void parseGameEntry(QXmlStreamReader&,
                    const Types::Collection&,
                    const QHash<QString, Types::Game*>&);
void applyMetadata(Types::Game&, const QHash<QString, QString>&);
void findAssets(Types::Game&,
                QHash<QString, QString>&,
                const Types::Collection&);
QHash<QString, QString>::iterator findByStrRef(QHash<QString, QString>&, const QStringRef&);
void convertToCanonicalPath(QString& path, const QString& containing_dir);

} // namespace es2_utils
} // namespace providers
