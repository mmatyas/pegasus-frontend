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


#include "Provider.h"

#include "model/gaming/Game.h"
#include "model/gaming/Collection.h"


namespace providers {
Provider::Provider(QLatin1String codename, QString display_name, uint8_t flags, QObject* parent)
    : QObject(parent)
    , m_codename(std::move(codename))
    , m_display_name(std::move(display_name))
    , m_flags(flags)
    , m_enabled(true)
{}

Provider::Provider(QLatin1String codename, QString display_name, QObject* parent)
    : Provider(std::move(codename), std::move(display_name), PROVIDER_FLAG_NONE, parent)
{}

Provider::~Provider() = default;

Provider& Provider::setEnabled(bool val)
{
    m_enabled = val;
    return *this;
}

Provider& Provider::setOption(const QString& key, QString val)
{
    Q_ASSERT(!key.isEmpty());
    Q_ASSERT(!val.isEmpty());
    setOption(key, std::vector<QString>{ std::move(val) });
    return *this;
}

Provider& Provider::setOption(const QString& key, std::vector<QString> vals)
{
    Q_ASSERT(!key.isEmpty());
    Q_ASSERT(!vals.empty());
    m_options[key] = std::move(vals);
    return *this;
}

} // namespace providers
