#include "Utils.h"

#include <QFileInfo>
#include <QString>
#include <QVector>

#ifdef Q_OS_UNIX
#include <sys/stat.h>
#endif


bool validFile(const QString& path) {
#ifdef Q_OS_UNIX
    // fast posix check for unix systems
    static struct ::stat buffer;
    return (::stat(path.toUtf8().constData(), &buffer) == 0);
#else
    // default Qt fallback
    QFileInfo file(path);
    return file.exists() && file.isFile();
#endif
}

void parseStoreInt(const QString &str, int &val) {
    bool success = false;
    const int parsed_val = str.toInt(&success);
    if (success)
        val = parsed_val;
}
