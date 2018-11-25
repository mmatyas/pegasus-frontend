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


#pragma once

#include "GameAssets.h"

#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <vector>


namespace modeldata {

struct Game;

struct Collection {
    explicit Collection(QString name);

    const QString& name() const { return m_name; }
    const QString& shortName() const { return m_short_name; }
    const QString& launchCmd() const { return m_launch_cmd; }
    void setShortName(const QString&);
    void setLaunchCmd(QString);

    QString launch_workdir;

    QString summary;
    QString description;

    GameAssets default_assets;


    Collection(const Collection&) = delete;
    Collection& operator=(const Collection&) = delete;
    Collection(Collection&&) = default;
    Collection& operator=(Collection&&) = default;

private:
    QString m_name;
    QString m_short_name;
    QString m_launch_cmd;
};

} // namespace modeldata
