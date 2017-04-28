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
    };

    static const QHash<Type, QVector<QString>> suffixes;

    static const QVector<QString>& extensions(const Type& key) {
        return key == Type::VIDEOS ? video_exts : image_exts;
    }

private:
    static const QVector<QString> image_exts;
    static const QVector<QString> video_exts;
};

inline uint qHash(const Assets::Type& key, uint seed) {
    return ::qHash(static_cast<unsigned char>(key), seed);
}
inline uint qHash(const Assets::Type& key) {
    return ::qHash(static_cast<unsigned char>(key));
}
