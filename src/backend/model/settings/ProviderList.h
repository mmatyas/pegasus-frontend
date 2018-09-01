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

#include <QObject>
#include <QQmlListProperty>
#include <QVector>


namespace model {

class Provider : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit Provider(QString name, bool* const value_ptr, QObject* parent = nullptr);

    const QString& name() const { return m_name; }

    bool enabled() const { return *m_value_ptr; }
    void setEnabled(bool);

signals:
    void enabledChanged();

private:
    const QString m_name;
    bool* const m_value_ptr;
};


class ProviderList : public QObject {
    Q_OBJECT

    Q_PROPERTY(int count
               READ count CONSTANT)
    Q_PROPERTY(QQmlListProperty<model::Provider> model
               READ getListProp CONSTANT)

public:
    explicit ProviderList(QObject* parent = nullptr);

    int count() const { return m_data.count(); }
    QQmlListProperty<Provider> getListProp();

signals:
    void currentChanged();

private:
    QVector<Provider*> m_data;
};

} // namespace model
