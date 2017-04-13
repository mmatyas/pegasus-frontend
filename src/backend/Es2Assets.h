#pragma once

#include <QString>
#include <QVector>

namespace Model { class Game; }
namespace Model { class Platform; }


class Es2Assets {
public:
    enum class AssetType : uint8_t {
        BOX_FRONT,
        LOGO,
        SCREENSHOT,
        VIDEO,
    };

    static QString find(AssetType, const Model::Platform*, const Model::Game*);

private:
    static QVector<QString> possibleSuffixes(AssetType);
    static QVector<QString> possibleExtensions(AssetType);
};
