// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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

#include <QString>
#include <QStringList>
#include <QRegularExpression>


namespace providers {
namespace pegasus {

enum class MetaAttribType : unsigned char;

class PegasusMetadata {
public:
    PegasusMetadata();

    void enhance_in_dirs(const QStringList&,
                         const HashMap<QString, modeldata::GamePtr>&,
                         const HashMap<QString, modeldata::Collection>&) const;

private:
    const HashMap<QString, MetaAttribType> m_key_types;
    const QRegularExpression m_player_regex;
    const QRegularExpression m_rating_percent_regex;
    const QRegularExpression m_rating_float_regex;
    const QRegularExpression m_release_regex;

    void read_metadata_file(const QString&, const HashMap<QString, modeldata::GamePtr>&) const;
};

} // namespace pegasus
} // namespace providers
