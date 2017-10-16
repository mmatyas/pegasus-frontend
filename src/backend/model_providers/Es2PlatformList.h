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

#include "PlatformListProvider.h"

#include <QString>

class QXmlStreamReader;


namespace model_providers {

class Es2PlatformList : public PlatformListProvider {
public:
    Es2PlatformList();

    QVector<Model::Platform*> find() final;

private:
    /// returns the path to the systems file, or an empty string
    QString findSystemsFile();

    /// processes the systems file, returns the list of successfully parsed platforms
    QVector<Model::Platform*> parseSystemsFile(QXmlStreamReader&);
    /// processes one `<system>` entry, returns a newly created Platform
    /// on success, or a nullptr on failure
    Model::Platform* parseSystemEntry(QXmlStreamReader&);
    /// returns a list of unique, '*.'-prefixed lowercase file extensions
    QStringList parseFilters(const QString&);

    const QVector<QString> required_system_props;
};

} // namespace model_providers
