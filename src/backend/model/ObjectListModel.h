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


namespace model {
class ObjectListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit ObjectListModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {}

    Q_INVOKABLE virtual QObject* get(int idx) const = 0;
    Q_INVOKABLE virtual bool isEmpty() const = 0;
    Q_INVOKABLE virtual int count() const = 0;

    Q_INVOKABLE virtual int indexOf(QObject* item) const = 0;
    Q_INVOKABLE bool contains(QObject* item) const { return indexOf(item) >= 0; }

    Q_INVOKABLE virtual QVariantList toVarArray() const = 0;

signals:
    void countChanged();
};


template<typename T>
class TypeListModel : public ObjectListModel {
public:
    explicit TypeListModel(QObject* parent)
        : ObjectListModel(parent)
    {}

    void update(std::vector<T*>&& entries) {
        const bool count_changed = m_entries.size() != entries.size();

        beginResetModel();
        for (T* entry : m_entries)
            QObject::disconnect(entry, nullptr, this, nullptr);

        m_entries = std::move(entries);

        for (T* entry : m_entries)
            connectEntry(entry);
        endResetModel();

        if (count_changed)
            emit countChanged();
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        return parent.isValid() ? 0 : m_entries.size();
    }

    QObject* get(int idx) const override {
        return (0 <= idx && static_cast<size_t>(idx) < m_entries.size())
            ? m_entries.at(idx)
            : nullptr;
    }

    int indexOf(QObject* item) const override {
        const auto it = std::find(m_entries.cbegin(), m_entries.cend(), item);
        return it == m_entries.cend()
            ? -1
            : std::distance(m_entries.cbegin(), it);
    }

    bool isEmpty() const override { return m_entries.empty(); }
    int count() const override { return m_entries.size(); }
    const std::vector<T*>& entries() const { return m_entries; }

    QVariantList toVarArray() const override {
        QVariantList varlist;
        varlist.reserve(m_entries.size());
        for (T* ptr : m_entries)
            varlist.append(QVariant::fromValue(ptr));
        return varlist;
    }

protected:
    virtual void connectEntry(T* const) {};

    std::vector<T*> m_entries;
};
} // namespace model
