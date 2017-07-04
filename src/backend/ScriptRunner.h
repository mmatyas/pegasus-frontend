// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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

#include <QMap>
#include <QString>
#include <QVector>


class ScriptRunner {
public:
    enum class EventType : unsigned char {
        QUIT,
        REBOOT,
        SHUTDOWN,
    };

    static void findAndRunSripts(const QVector<EventType>&);
    static void findAndRunSripts(EventType);

    static QVector<QString> findScripts(const QString& dirname);
    static void runScripts(const QVector<QString>& paths);

private:
    static const QMap<EventType, QString> script_dirs;
};
