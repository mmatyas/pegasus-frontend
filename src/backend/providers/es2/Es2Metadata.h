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
#include <QObject>
#include <QRegularExpression>
#include <QXmlStreamReader>

namespace Types { class Collection; }
namespace Types { class Game; }


namespace providers {
namespace es2 {

enum class MetaTypes : unsigned char;

class MetadataParser : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(MetadataParser)

public:
    MetadataParser(QObject* parent);
    void enhance(const QHash<QString, Types::Game*>& games,
                 const QHash<QString, Types::Collection*>& collections);

private:
    const QHash<QString, MetaTypes> m_key_types;
    const QString m_date_format;
    const QRegularExpression m_players_regex;

    void parseGamelistFile(QXmlStreamReader&,
                           const Types::Collection&,
                           const QHash<QString, Types::Game*>&) const;
    void parseGameEntry(QXmlStreamReader&,
                        const Types::Collection&,
                        const QHash<QString, Types::Game*>&) const;
    void applyMetadata(Types::Game&, const QHash<MetaTypes, QString>&) const;
};

} // namespace es2
} // namespace providers
