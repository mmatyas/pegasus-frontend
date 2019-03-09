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


#include "PegasusParser.h"

#include "LocaleUtils.h"

#include <QDebug>


namespace providers {
namespace pegasus {
namespace parser {

ParserContext::ParserContext(QString file_path, const Constants& constants)
    : metafile_path(std::move(file_path))
    , dir_path(QFileInfo(metafile_path).path())
    , constants(constants)
    , cur_coll(nullptr)
    , cur_filter(nullptr)
    , cur_game(nullptr)
{
    Q_ASSERT(!metafile_path.isEmpty());
    Q_ASSERT(!dir_path.isEmpty());
}

void ParserContext::print_error(const int lineno, const QString msg) const {
    qWarning().noquote()
        << tr_log("Collections: `%1`, line %2: %3").arg(metafile_path, QString::number(lineno), msg);
}

} // namespace parser
} // namespace pegasus
} // namespace providers
