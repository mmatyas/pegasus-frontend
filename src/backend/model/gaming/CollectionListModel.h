// Pegasus Frontend
// Copyright (C) 2017-2022  Mátyás Mustoha
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

#include <QAbstractListModel>
#include <QVariantList>

namespace model { class Collection; }


namespace model {
class CollectionListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit CollectionListModel(QObject* parent = nullptr);
    CollectionListModel& update(std::vector<model::Collection*>&&);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    const std::vector<model::Collection*>& entries() const { return m_entries; }
    int count() const { return m_entries.size(); }

    Q_INVOKABLE QVariantList toVarArray() const;
    Q_INVOKABLE model::Collection* get(int idx) const;

signals:
    void countChanged();

private:
    std::vector<model::Collection*> m_entries;
};
} // namespace model
