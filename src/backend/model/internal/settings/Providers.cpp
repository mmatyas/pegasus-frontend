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
#include "providers/Provider.h"


namespace model {
ProviderEntry::ProviderEntry(const size_t idx)
    : m_idx(idx)
{}

bool ProviderEntry::enabled() const {
    return AppSettings::providers.at(m_idx)->enabled();
}

void ProviderEntry::setEnabled(bool value) {
    AppSettings::providers.at(m_idx)->setEnabled(value);
}

const QString& ProviderEntry::name() const {
    return AppSettings::providers.at(m_idx)->name();
}


Providers::Providers(QObject* parent)
    : QAbstractListModel(parent)
    , m_role_names({
        { Roles::Name, QByteArrayLiteral("name") },
        { Roles::Enabled, QByteArrayLiteral("enabled") },
    })
{
    for (size_t i = 0; i < AppSettings::providers.size(); i++) {
        const auto& ptr = AppSettings::providers.at(i);
        if ((ptr->flags() & providers::INTERNAL) == 0)
            m_providers.emplace_back(i);
    }
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
            return provider.name();
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
