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


#include "Providers.h"

#include "AppSettings.h"


namespace model {
ProviderEntry::ProviderEntry(ExtProvider id, QString name)
    : name(std::move(name))
    , m_id(id)
{}

bool ProviderEntry::enabled() const
{
    return AppSettings::ext_providers.enabled(m_id);
}

void ProviderEntry::setEnabled(bool value)
{
    AppSettings::ext_providers.enabled_mut(m_id) = value;
}


Providers::Providers(QObject* parent)
    : QAbstractListModel(parent)
    , m_role_names({
        { Roles::Name, QByteArrayLiteral("name") },
        { Roles::Enabled, QByteArrayLiteral("enabled") },
    })
{
#ifdef WITH_COMPAT_ES2
    m_providers.emplace_back(ExtProvider::ES2, QStringLiteral("EmulationStation"));
#endif
#ifdef WITH_COMPAT_STEAM
    m_providers.emplace_back(ExtProvider::STEAM, QStringLiteral("Steam"));
#endif
#ifdef WITH_COMPAT_GOG
    m_providers.emplace_back(ExtProvider::GOG, QStringLiteral("GOG"));
#endif
#ifdef WITH_COMPAT_ANDROIDAPPS
    m_providers.emplace_back(ExtProvider::ANDROIDAPPS, QStringLiteral("Android Apps"));
#endif
#ifdef WITH_COMPAT_SKRAPER
    m_providers.emplace_back(ExtProvider::SKRAPER, QStringLiteral("Skraper Assets"));
#endif

}

int Providers::count() const
{
    return static_cast<int>(m_providers.size());
}

int Providers::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return count();
}

QVariant Providers::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || rowCount() <= index.row())
        return {};

    const auto& provider = m_providers.at(static_cast<size_t>(index.row()));
    switch (role) {
        case Roles::Name:
            return provider.name;
        case Roles::Enabled:
            return provider.enabled();
        default:
            return {};
    }
}

bool Providers::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (data(index, role) == value)
        return false;

    auto& provider = m_providers.at(static_cast<size_t>(index.row()));
    switch (role) {
        case Roles::Enabled:
            provider.setEnabled(value.toBool());
            break;
        default:
            return false;
    }

    emit dataChanged(index, index, QVector<int>() << role);
    AppSettings::save_config();
    return true;
}

Qt::ItemFlags Providers::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | Qt::ItemNeverHasChildren | QAbstractListModel::flags(index);
}
} // namespace model
