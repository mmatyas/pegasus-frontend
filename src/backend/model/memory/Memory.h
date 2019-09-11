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
#include <QVariantMap>


namespace model {
class Memory : public QObject {
    Q_OBJECT

public:
    explicit Memory(QObject* parent = nullptr);
    explicit Memory(QString settings_dir, QObject* parent = nullptr);

    Q_INVOKABLE QVariant get(const QString&) const;
    Q_INVOKABLE bool has(const QString&) const;
    Q_INVOKABLE void set(const QString&, QVariant);
    Q_INVOKABLE void unset(const QString&);

    void changeTheme(const QString&);

signals:
    // NOTE: because QVariantMap cannot be changed on the QML side (QTBUG-59474),
    // get/set functions were introduced. Because of this however, sending a
    // notify signal to the QML engine is required when a change happens.
    void dataChanged();

private:
    const QString m_settings_dir;
    QString m_current_theme;
    QVariantMap m_data;

    void flush() const;
};
} // namespace model
