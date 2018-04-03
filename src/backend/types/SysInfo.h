// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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

#include <QObject>


namespace Types {

class SystemInfo : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString arch READ arch CONSTANT)
    Q_PROPERTY(QString kernel READ arch CONSTANT)
    Q_PROPERTY(QString product READ product CONSTANT)

public:
    explicit SystemInfo(QObject* parent = nullptr);

    QString arch() const;
    QString kernel() const;
    QString product() const;
};

} // namespace Types
