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

#include "utils/FwdDeclModelData.h"
#include "utils/HashMap.h"

#include <QObject>
#include <QRegularExpression>
#include <QXmlStreamReader>


namespace providers {
namespace es2 {

enum class MetaTypes : unsigned char;

class MetadataParser : public QObject {
    Q_OBJECT

public:
    MetadataParser(QObject* parent);
    void enhance(HashMap<QString, modeldata::Game>& games,
                 const HashMap<QString, modeldata::Collection>& collections,
                 const HashMap<QString, std::vector<QString>>& collection_childs,
                 const HashMap<QString, QString>& collection_dirs);

private:
    const HashMap<QString, MetaTypes> m_key_types;
    const QString m_date_format;
    const QRegularExpression m_players_regex;

    void parseGamelistFile(QXmlStreamReader&,
                           HashMap<QString, modeldata::Game>&,
                           const QString&) const;
    void parseGameEntry(QXmlStreamReader&,
                        HashMap<QString, modeldata::Game>&,
                        const QString&) const;
    void applyMetadata(modeldata::Game&,
                       HashMap<MetaTypes, QString, EnumHash>&) const;
};

} // namespace es2
} // namespace providers
