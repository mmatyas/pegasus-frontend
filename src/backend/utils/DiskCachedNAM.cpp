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


#include "DiskCachedNAM.h"

#include "Paths.h"

#include <QNetworkAccessManager>
#include <QNetworkDiskCache>


namespace utils {

QNetworkAccessManager* create_disc_cached_nam(QObject* parent)
{
    auto nam = new QNetworkAccessManager(parent);
    auto cache = new QNetworkDiskCache(nam);

    QString cache_path = paths::writableCacheDir() + QLatin1String("/netcache");
    cache->setCacheDirectory(cache_path);

    nam->setCache(cache);
    return nam;
}

} // namespace utils
