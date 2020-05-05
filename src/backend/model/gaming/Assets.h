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


#pragma once

#include "types/AssetType.h"
#include "utils/HashMap.h"
#include "utils/MoveOnly.h"

#include <QStringList>
#include <QObject>


namespace model {
class Assets : public QObject {
    Q_OBJECT

public:
    // NOTE: by manually listing the properties (instead of eg. a Map),
    //       it is also possible to refer the same data by a different name
    // TODO: these could be optimized, see
    //       https://doc.qt.io/qt-5/qtqml-cppintegration-data.html (Sequence Type to JavaScript Array)
#define GEN(qmlname, enumname) \
    const QString& qmlname() const { return getFirst(AssetType::enumname); } \
    const QStringList& qmlname##List() const { return get(AssetType::enumname); } \
    Q_PROPERTY(QString qmlname READ qmlname CONSTANT) \
    Q_PROPERTY(QStringList qmlname##List READ qmlname##List CONSTANT) \

    GEN(boxFront, BOX_FRONT)
    GEN(boxBack, BOX_BACK)
    GEN(boxSpine, BOX_SPINE)
    GEN(boxFull, BOX_FULL)
    GEN(cartridge, CARTRIDGE)
    GEN(logo, LOGO)
    GEN(poster, POSTER)

    GEN(marquee, ARCADE_MARQUEE)
    GEN(bezel, ARCADE_BEZEL)
    GEN(panel, ARCADE_PANEL)
    GEN(cabinetLeft, ARCADE_CABINET_L)
    GEN(cabinetRight, ARCADE_CABINET_R)

    GEN(tile, UI_TILE)
    GEN(banner, UI_BANNER)
    GEN(steam, UI_STEAMGRID)
    GEN(background, BACKGROUND)
    GEN(music, MUSIC)

    GEN(screenshot, SCREENSHOT)
    GEN(video, VIDEO)
#undef GEN

public:
    explicit Assets(QObject* parent);

    void add_file(AssetType, QString);
    void add_url(AssetType, QString);

private:
    const QStringList& get(AssetType) const;
    const QString& getFirst(AssetType) const;

    HashMap<AssetType, QStringList, EnumHash> m_asset_lists;
};

} // namespace model
