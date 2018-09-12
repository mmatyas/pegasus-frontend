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


#include "ProviderList.h"

#include "AppSettings.h"
#include "ListPropertyFn.h"

#include <QDebug>


namespace model {

Provider::Provider(QString name, bool* const value_ptr, QObject* parent)
    : QObject(parent)
    , m_name(std::move(name))
    , m_value_ptr(value_ptr)
{}

void Provider::setEnabled(bool val)
{
    if (val == enabled())
        return;

    *m_value_ptr = val;
    enabledChanged();

    AppSettings::save_config();
}


ProviderList::ProviderList(QObject* parent)
    : QObject(parent)
{
#ifdef WITH_COMPAT_ES2
    m_data.append(new model::Provider(QStringLiteral("EmulationStation"),
                                      &AppSettings::enable_provider_es2,
                                      this));
#endif
#ifdef WITH_COMPAT_STEAM
    m_data.append(new model::Provider(QStringLiteral("Steam"),
                                      &AppSettings::enable_provider_steam,
                                      this));
#endif
#ifdef WITH_COMPAT_GOG
    m_data.append(new model::Provider(QStringLiteral("GOG"),
                                      &AppSettings::enable_provider_gog,
                                      this));
#endif
}

QQmlListProperty<Provider> ProviderList::getListProp()
{
    static constexpr auto count = &listproperty_count<Provider>;
    static constexpr auto at = &listproperty_at<Provider>;

    return {this, &m_data, count, at};
}

} // namespace model
