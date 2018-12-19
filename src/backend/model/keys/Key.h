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

#include <QObject>


namespace model {
class Key : public QObject {
    Q_OBJECT
    Q_PROPERTY(int key READ key CONSTANT)
    Q_PROPERTY(int modifiers READ modifiers CONSTANT)
    Q_PROPERTY(int keyCode READ keyCode CONSTANT)

public:
    explicit Key(QObject* parent = nullptr);
    explicit Key(const QKeySequence&, QObject* parent = nullptr);

    int key() const { return m_key; }
    int modifiers() const { return m_modifiers; }
    int keyCode() const { return m_key + m_modifiers; }

    Q_INVOKABLE QString name() const;

private:
    const int m_modifiers;
    const int m_key;
};
} // namespace model
