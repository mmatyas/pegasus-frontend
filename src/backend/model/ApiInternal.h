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

#include "model/general/Meta.h"
#include "model/general/System.h"
#include "model/settings/Settings.h"
#include <QObject>


#define GET_PTR(type, name) \
    type* name##Ptr() { return &name; }


namespace model {
class ApiInternal : public QObject {
    Q_OBJECT

    Q_PROPERTY(model::Meta* meta READ metaPtr CONSTANT)
    Q_PROPERTY(model::Settings* settings READ settingsPtr CONSTANT)
    Q_PROPERTY(model::System* system READ systemPtr CONSTANT)

public:
    explicit ApiInternal();

    GET_PTR(model::Meta, meta)
    GET_PTR(model::Settings, settings)
    GET_PTR(model::System, system)

public:
    model::Meta meta;
    model::System system;
    model::Settings settings;
};
} // namespace model


#undef GET_PTR
