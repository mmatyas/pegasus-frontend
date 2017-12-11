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
#include <functional>

namespace Types { class Collection; }
namespace Types { class Game; }


namespace providers {
namespace es2_utils {

void readSystemsFile(QXmlStreamReader& xml,
                     QHash<QString, Types::Game*>& games,
                     QHash<QString, Types::Collection*>& collections,
                     QVector<QString>& metadata_dirs,
                     const std::function<void(int)>& onGamesChangedCB);
void readSystemEntry(QXmlStreamReader& xml,
                     QHash<QString, Types::Game*>& games,
                     QHash<QString, Types::Collection*>& collections,
                     QVector<QString>& metadata_dirs);
QHash<QLatin1String, QString>::iterator findByStrRef(QHash<QLatin1String, QString>&, const QStringRef&);
QStringList parseFilters(const QString& filters_raw);

} // namespace es2_utils
} // namespace providers
