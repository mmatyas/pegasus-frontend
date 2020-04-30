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

#include "LaunchBoxCommon.h"
#include "providers/Provider.h"


namespace providers {
namespace launchbox {
namespace gamelist_xml {

void read(
    const providers::Provider* const provider,
    const Literals& literals,
    const QString& lb_dir,
    const QString& platform_name,
    const HashMap<EmulatorId, Emulator>& emulators,
    providers::SearchContext& sctx);

} // namespace gamelist_xml
} // namespace launchbox
} // namespace providers
