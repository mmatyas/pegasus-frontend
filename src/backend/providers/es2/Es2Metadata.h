// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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

#include "providers/Provider.h"
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
    void enhance(providers::SearchContext& sctx,
                 const HashMap<QString, QString>& collection_dirs);

private:
    const HashMap<QString, MetaTypes> m_key_types;
    const QString m_date_format;
    const QRegularExpression m_players_regex;

    void parseGamelistFile(QXmlStreamReader&,
                           providers::SearchContext&,
                           const QString&) const;
    void parseGameEntry(QXmlStreamReader&,
                        providers::SearchContext&,
                        const QString&) const;
    void applyMetadata(model::Game&,
                       HashMap<MetaTypes, QString, EnumHash>&) const;
};

} // namespace es2
} // namespace providers
