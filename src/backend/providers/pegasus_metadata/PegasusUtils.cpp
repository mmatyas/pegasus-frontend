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


#include "PegasusUtils.h"

#include <QFileInfo>
#include <QStringBuilder>
#include <QUrl>


namespace providers {
namespace pegasus {
namespace utils {

QStringList tokenize_by_comma(const QString& str)
{
    QStringList list = str.split(QChar(','), QString::SkipEmptyParts);
    for (QString& item : list)
        item = item.trimmed();

    return list;
}

QString assetline_to_url(const QString& value, const QString& relative_dir)
{
    Q_ASSERT(!value.isEmpty());
    Q_ASSERT(!relative_dir.isEmpty());

    if (value.startsWith(QLatin1String("http://")) || value.startsWith(QLatin1String("https://")))
        return value;

    QFileInfo finfo(value);
    if (finfo.isRelative())
        finfo.setFile(relative_dir % '/' % value);

    return QUrl::fromLocalFile(finfo.absoluteFilePath()).toString();
}

} // namespace utils
} // namespace pegasus
} // namespace providers
