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

#include "Es2Systems.h"
#include "Es2Metadata.h"
#include "providers/Provider.h"
#include "utils/HashMap.h"


namespace providers {
namespace es2 {

class Es2Provider : public Provider {
    Q_OBJECT

public:
    Es2Provider(QObject* parent = nullptr);

    void findLists(HashMap<QString, modeldata::Game>&,
                   HashMap<QString, modeldata::Collection>&,
                   HashMap<QString, std::vector<QString>>&) final;
    void findStaticData(HashMap<QString, modeldata::Game>&,
                        const HashMap<QString, modeldata::Collection>&,
                        const HashMap<QString, std::vector<QString>>&) final;

private:
    SystemsParser systems;
    MetadataParser metadata;
    HashMap<QString, QString> m_collection_dirs;
};

} // namespace es2
} // namespace providers
