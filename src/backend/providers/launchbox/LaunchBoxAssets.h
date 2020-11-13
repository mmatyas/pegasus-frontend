// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include "utils/HashMap.h"

#include <QString>
#include <QRegularExpression>
#include <vector>

namespace model { class Game; }
enum class AssetType : unsigned char;
class QString;


namespace providers {
namespace launchbox {

class Assets {
public:
    explicit Assets(QString, QString);

    void find_assets_for(const QString&, const std::vector<model::Game*>&) const;

private:
    const QString m_log_tag;
    const QString m_lb_root_path;

    const std::vector<std::pair<QString, AssetType>> m_dir_list;
    const QRegularExpression rx_number_suffix;

    void find_assets_in(const QString&, const AssetType, const HashMap<QString, model::Game*>&) const;
};

} // namespace launchbox
} // namespace providers
