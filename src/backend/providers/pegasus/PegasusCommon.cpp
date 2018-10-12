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


#include "PegasusCommon.h"

#include <QFileInfo>
#include <QUrl>


namespace providers {
namespace pegasus {

QStringList tokenize(const QString& str)
{
    QStringList list = str.split(',', QString::SkipEmptyParts);
    for (QString& item : list)
        item = item.trimmed();

    return list;
}

void add_asset(modeldata::GameAssets& game_assets, const AssetType asset_type, const QString& value,
               const QString& relative_dir)
{
    Q_ASSERT(asset_type != AssetType::UNKNOWN);

    QString url;
    if (value.startsWith(QStringLiteral("http://")) || value.startsWith(QStringLiteral("https://"))) {
        url = value;
    }
    else {
        QFileInfo finfo(value);
        if (finfo.isRelative())
            finfo.setFile(relative_dir + '/' + value);

        url = QUrl::fromLocalFile(finfo.absoluteFilePath()).toString();
    }

    game_assets.addUrlMaybe(asset_type, std::move(url));
}

} // namespace pegasus
} // namespace providers
