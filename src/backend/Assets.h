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
#include <QVector>


enum class AssetType : unsigned char {
    BOX_FRONT,
    BOX_BACK,
    BOX_SPINE,
    BOX_FULL,
    CARTRIDGE,
    LOGO, // just the game logo
    MARQUEE, // game logo with background
    BEZEL, // image around the game screen
    STEAMGRID,
    FLYER,
    FANARTS,
    SCREENSHOTS,
    VIDEOS,
    MUSIC,
};

/// Definitions of supported asset types and file formats
class Assets {
public:
    static const QVector<AssetType> singleTypes;
    static const QVector<AssetType> multiTypes;
    static const QHash<AssetType, QVector<QString>> suffixes;

    static const QVector<QString>& extensions(AssetType key);

    static QString findFirst(AssetType, const QString& path_base);
    static QStringList findAll(AssetType, const QString& path_base);
};

inline uint qHash(const AssetType& key, uint seed) {
    return ::qHash(static_cast<unsigned char>(key), seed);
}
inline uint qHash(const AssetType& key) {
    return ::qHash(static_cast<unsigned char>(key));
}
