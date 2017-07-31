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

#include "MetadataProvider.h"

class QXmlStreamReader;


namespace model_providers {

class Es2Metadata : public MetadataProvider {
public:
    Es2Metadata();

    void fill(const Model::Platform&) final;

private:
    /// returns the path to the metadata file, or an empty string
    QString findGamelistFile(const Model::Platform&);

    /// processes the metadata file, filling up the games in the process
    void parseGamelistFile(QXmlStreamReader&, const Model::Platform&);
    /// processes one <game> node, checks if there is a matching game,
    /// and fills its entry with metadata on success
    void parseGameEntry(QXmlStreamReader&, const Model::Platform&,
                        QHash<QString, Model::Game*>&);

    /// fills a game with metadata
    void applyMetadata(Model::Game&, const Model::Platform&, const QHash<QString, QString>&);

    /// replaces leading `~` with the home dir and makes paths starting
    /// with dot(s) be relative to a root dir
    void convertToAbsolutePath(QString& path, const QString& root_dir_prefix);
};

} // namespace model_providers
