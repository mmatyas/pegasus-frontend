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


/// Definitions of supported asset types and file formats
class Assets {
public:
    enum class Type : unsigned char {
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

    static const QVector<Assets::Type> singleTypes;
    static const QVector<Assets::Type> multiTypes;
    static const QHash<Type, QVector<QString>> suffixes;

    static const QVector<QString>& extensions(Type key);

    static QString findFirst(Assets::Type, const QString& path_base);
    static QStringList findAll(Assets::Type, const QString& path_base);

private:
    static const QVector<QString> m_image_exts;
    static const QVector<QString> m_video_exts;
    static const QVector<QString> m_audio_exts;
};

inline uint qHash(const Assets::Type& key, uint seed) {
    return ::qHash(static_cast<unsigned char>(key), seed);
}
inline uint qHash(const Assets::Type& key) {
    return ::qHash(static_cast<unsigned char>(key));
}
