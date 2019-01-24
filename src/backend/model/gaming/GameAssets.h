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

#include "modeldata/gaming/GameAssetsData.h"

#include <QObject>


#define SINGLE_ASSET_PROP(api_name, asset_type) \
    Q_PROPERTY(QString api_name READ api_name CONSTANT) \
    const QString& api_name() const { return m_assets->single(AssetType::asset_type); }


namespace model {

class GameAssets : public QObject {
    Q_OBJECT

    // NOTE: by manually listing the properties (instead of eg. a Map),
    //       it is also possible to refer the same data by a different name

    SINGLE_ASSET_PROP(boxFront, BOX_FRONT)
    SINGLE_ASSET_PROP(boxBack, BOX_BACK)
    SINGLE_ASSET_PROP(boxSpine, BOX_SPINE)
    SINGLE_ASSET_PROP(boxFull, BOX_FULL)
    SINGLE_ASSET_PROP(cartridge, CARTRIDGE)
    SINGLE_ASSET_PROP(logo, LOGO)
    SINGLE_ASSET_PROP(poster, POSTER)

    SINGLE_ASSET_PROP(marquee, ARCADE_MARQUEE)
    SINGLE_ASSET_PROP(bezel, ARCADE_BEZEL)
    SINGLE_ASSET_PROP(panel, ARCADE_PANEL)
    SINGLE_ASSET_PROP(cabinetLeft, ARCADE_CABINET_L)
    SINGLE_ASSET_PROP(cabinetRight, ARCADE_CABINET_R)

    SINGLE_ASSET_PROP(tile, UI_TILE)
    SINGLE_ASSET_PROP(banner, UI_BANNER)
    SINGLE_ASSET_PROP(steam, UI_STEAMGRID)
    SINGLE_ASSET_PROP(background, BACKGROUND)
    SINGLE_ASSET_PROP(music, MUSIC)

    // TODO: these could be optimized, see
    // https://doc.qt.io/qt-5/qtqml-cppintegration-data.html (Sequence Type to JavaScript Array)
    Q_PROPERTY(QStringList screenshots READ screenshots CONSTANT)
    Q_PROPERTY(QStringList videos READ videos CONSTANT)

public:
    explicit GameAssets(modeldata::GameAssets* const, QObject* parent = nullptr);

private:
    const QStringList& screenshots() { return m_assets->multi(AssetType::SCREENSHOTS); }
    const QStringList& videos() { return m_assets->multi(AssetType::VIDEOS); }

private:
    modeldata::GameAssets* const m_assets;
};

} // namespace model
