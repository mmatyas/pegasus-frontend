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

#include "utils/NoCopyNoMove.h"

#include <QString>
#include <memory>
#include <vector>


class LogSink {
public:
    LogSink();
    virtual ~LogSink();
    NO_COPY_NO_MOVE(LogSink)

    virtual void info(const QString&) = 0;
    virtual void warning(const QString&) = 0;
    virtual void error(const QString&) = 0;
};


class Log {
public:
    Log() = delete;
    NO_COPY_NO_MOVE(Log)

    static void init(bool silent);
    static void close();

    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);

private:
    static std::vector<std::unique_ptr<LogSink>> m_sinks;
};
