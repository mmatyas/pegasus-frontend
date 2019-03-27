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

#include "providers/pegasus_favorites/Favorites.h"
#include "providers/pegasus_metadata/PegasusProvider.h"
#include "providers/pegasus_playtime/PlaytimeStats.h"

#ifdef WITH_COMPAT_ES2
  #include "providers/es2/Es2Provider.h"
#endif

#ifdef WITH_COMPAT_STEAM
  #include "providers/steam/SteamProvider.h"
#endif

#ifdef WITH_COMPAT_GOG
  #include "providers/gog/GogProvider.h"
#endif

#ifdef WITH_COMPAT_ANDROIDAPPS
  #include "providers/android_apps/AndroidAppsProvider.h"
#endif

#ifdef WITH_COMPAT_SKRAPER
  #include "providers/skraper/SkraperAssetsProvider.h"
#endif

// TODO: return std::vector<Provider*>
