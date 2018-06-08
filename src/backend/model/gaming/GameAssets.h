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

#include "modeldata/gaming/GameAssets.h"

#include <QObject>


namespace model {

class GameAssets : public QObject {
    Q_OBJECT

    // NOTE: by manually listing the properties (instead of eg. a Map),
    //       it is also possible to refer the same data by a different name

    Q_PROPERTY(QString boxFront READ boxFront CONSTANT)
    Q_PROPERTY(QString boxBack READ boxBack CONSTANT)
    Q_PROPERTY(QString boxSpine READ boxSpine CONSTANT)
    Q_PROPERTY(QString boxFull READ boxFull CONSTANT)
    Q_PROPERTY(QString cartridge READ cartridge CONSTANT)
    Q_PROPERTY(QString logo READ logo CONSTANT)
    Q_PROPERTY(QString poster READ poster CONSTANT)

    Q_PROPERTY(QString marquee READ marquee CONSTANT)
    Q_PROPERTY(QString bezel READ bezel CONSTANT)
    Q_PROPERTY(QString panel READ panel CONSTANT)
    Q_PROPERTY(QString cabinetLeft READ cabinetLeft CONSTANT)
    Q_PROPERTY(QString cabinetRight READ cabinetRight CONSTANT)

    Q_PROPERTY(QString tile READ tile CONSTANT)
    Q_PROPERTY(QString banner READ banner CONSTANT)
    Q_PROPERTY(QString steam READ steam CONSTANT)
    Q_PROPERTY(QString background READ background CONSTANT)
    Q_PROPERTY(QString music READ music CONSTANT)

    // TODO: these could be optimized, see
    // https://doc.qt.io/qt-5/qtqml-cppintegration-data.html (Sequence Type to JavaScript Array)
    Q_PROPERTY(QStringList screenshots READ screenshots CONSTANT)
    Q_PROPERTY(QStringList videos READ videos CONSTANT)

public:
    explicit GameAssets(modeldata::GameAssets* const, QObject* parent = nullptr);

    const QString& boxFront() { return m_assets->single(AssetType::BOX_FRONT); }
    const QString& boxBack() { return m_assets->single(AssetType::BOX_BACK); }
    const QString& boxSpine() { return m_assets->single(AssetType::BOX_SPINE); }
    const QString& boxFull() { return m_assets->single(AssetType::BOX_FULL); }
    const QString& cartridge() { return m_assets->single(AssetType::CARTRIDGE); }
    const QString& logo() { return m_assets->single(AssetType::LOGO); }
    const QString& poster() { return m_assets->single(AssetType::POSTER); }

    const QString& marquee() { return m_assets->single(AssetType::ARCADE_MARQUEE); }
    const QString& bezel() { return m_assets->single(AssetType::ARCADE_BEZEL); }
    const QString& panel() { return m_assets->single(AssetType::ARCADE_PANEL); }
    const QString& cabinetLeft() { return m_assets->single(AssetType::ARCADE_CABINET_L); }
    const QString& cabinetRight() { return m_assets->single(AssetType::ARCADE_CABINET_R); }

    const QString& tile() { return m_assets->single(AssetType::UI_TILE); }
    const QString& banner() { return m_assets->single(AssetType::UI_BANNER); }
    const QString& steam() { return m_assets->single(AssetType::UI_STEAMGRID); }
    const QString& background() { return m_assets->single(AssetType::BACKGROUND); }
    const QString& music() { return m_assets->single(AssetType::MUSIC); }

    const QStringList& screenshots() { return m_assets->multi(AssetType::SCREENSHOTS); }
    const QStringList& videos() { return m_assets->multi(AssetType::VIDEOS); }

private:
    modeldata::GameAssets* const m_assets;
};

} // namespace model
