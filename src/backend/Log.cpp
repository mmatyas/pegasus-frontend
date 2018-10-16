#include "Log.h"

#include "AppSettings.h"
#include "LocaleUtils.h"
#include "Paths.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>


LogSink::LogSink() = default;
LogSink::~LogSink() = default;


namespace logsinks {

class Terminal : public LogSink {
public:
    Terminal()
        : m_stream(stdout)
    {}

    void info(const QString& msg) override {
        colorlog(m_pre_info, msg);
    }
    void warning(const QString& msg) override {
        colorlog(m_pre_warning, msg);
    }
    void error(const QString& msg) override {
        colorlog(m_pre_error, msg);
    }

private:
    QTextStream m_stream;

#ifdef Q_OS_WIN
    static constexpr auto m_pre_info = "[i]";
    static constexpr auto m_pre_warning = "[w]";
    static constexpr auto m_pre_error = "[e]";
    static constexpr auto m_fmt_reset = "";
#else
    static constexpr auto m_pre_info = "[i]";
    static constexpr auto m_pre_warning = "\x1b[93m[w]";
    static constexpr auto m_pre_error = "\x1b[91m[e]";
    static constexpr auto m_fmt_reset = "\x1b[0m";
#endif

    void colorlog(const char* const prefix, const QString& msg) {
        m_stream << prefix << QChar(' ') << msg << m_fmt_reset << endl;
    }
};


class LogFile : public LogSink {
public:
    LogFile()
        : m_file(default_log_path())
    {
        if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            Log::warning(tr_log("Could not open `%1` for writing, file logging disabled.")
                         .arg(m_file.fileName()));
            return;
        }

        m_stream.setDevice(&m_file);
    }

    void info(const QString& msg) override {
        datelog(m_marker_info, msg);
    }
    void warning(const QString& msg) override {
        datelog(m_marker_warning, msg);
        m_stream.flush();
    }
    void error(const QString& msg) override {
        datelog(m_marker_error, msg);
        m_stream.flush();
    }

private:
    QFile m_file;
    QTextStream m_stream;

    static constexpr auto m_marker_info = "[i]";
    static constexpr auto m_marker_warning = "[w]";
    static constexpr auto m_marker_error = "[e]";

    QString default_log_path() {
        return paths::writableConfigDir() + QLatin1String("/lastrun.log");
    }

    void datelog(const char* const marker, const QString& msg) {
        m_stream << QDateTime::currentDateTime().toString(Qt::ISODate) << QChar(' ')
                 << marker << QChar(' ')
                 << msg << QChar('\n');
    }
};

} // namespace logsinks


namespace {

void on_qt_message(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    const QString prepared_msg = qFormatLogMessage(type, context, msg);
    switch (type) {
        case QtMsgType::QtDebugMsg:
        case QtMsgType::QtInfoMsg:
            Log::info(prepared_msg);
            break;
        case QtMsgType::QtWarningMsg:
            Log::warning(prepared_msg);
            break;
        case QtMsgType::QtCriticalMsg:
        case QtMsgType::QtFatalMsg:
            Log::error(prepared_msg);
            break;
        default:
            Q_UNREACHABLE();
            break;
    }
}

} // namespace


std::vector<std::unique_ptr<LogSink>> Log::m_sinks {};

void Log::init()
{
    if (!AppSettings::general.silent)
        m_sinks.emplace_back(new logsinks::Terminal());

    m_sinks.emplace_back(new logsinks::LogFile());

    // redirect Qt messages to the Log too
    qInstallMessageHandler(on_qt_message);
}

#define FORALLSINK_CALLER(method) \
    void Log::method(const QString& message) \
    { \
        for (const auto& sink : m_sinks) \
            sink->method(message); \
    }
FORALLSINK_CALLER(info)
FORALLSINK_CALLER(warning)
FORALLSINK_CALLER(error)
