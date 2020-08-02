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

#include "utils/MoveOnly.h"

#include <QAbstractListModel>


namespace model {
struct ProviderEntry {
    ProviderEntry(size_t idx);

    bool enabled() const;
    void setEnabled(bool);
    const QString& name() const;

private:
    const size_t m_idx;
};


class Providers : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count CONSTANT)

public:
    explicit Providers(QObject* parent = nullptr);

    enum Roles {
        Name = Qt::UserRole + 1,
        Enabled,
    };

    int count() const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override { return m_role_names; }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

private:
    const QHash<int, QByteArray> m_role_names;
    std::vector<ProviderEntry> m_providers;
};
} // namespace model
