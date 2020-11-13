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

#include <QDir>
#include <QString>

namespace model { class Collection; }
namespace model { class Game; }
namespace providers { class SearchContext; }
class QXmlStreamReader;


namespace providers {
namespace launchbox {

enum class GameField : unsigned char;
enum class AppField : unsigned char;
struct Emulator;

class GamelistXml {
public:
    explicit GamelistXml(QString, QDir);

    std::vector<model::Game*> find_games_for(const QString&, const HashMap<QString, Emulator>&, SearchContext&) const;

private:
    const QString m_log_tag;
    const QDir m_lb_root;
    const HashMap<QString, GameField> m_game_keys;
    const HashMap<QString, AppField> m_app_keys;

    void log_xml_warning(const QString&, const size_t, const QString&) const;
    HashMap<GameField, QString> read_game_node(QXmlStreamReader&) const;
    HashMap<AppField, QString> read_app_node(QXmlStreamReader&) const;
    bool game_fields_valid(const QString&, const size_t, const HashMap<GameField, QString>&, const HashMap<QString, Emulator>&) const;
    bool app_fields_valid(const QString&, const size_t, const HashMap<AppField, QString>&) const;
};

} // namespace launchbox
} // namespace providers
