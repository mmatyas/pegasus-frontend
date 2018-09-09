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


#include <QString>


namespace providers {
namespace gog {

QString gog_id_key()
{
    return QStringLiteral("gog.id");
}

QString json_api_suffix()
{
    return QStringLiteral("_api");
}

QString json_embed_suffix()
{
    return QStringLiteral("_embed");
}

} // namespace gog
} // namespace providers
