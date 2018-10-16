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

    static void init();

    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);

private:
    static std::vector<std::unique_ptr<LogSink>> m_sinks;
};
