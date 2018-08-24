#include "GlobalSettings.h"

#include "LocaleUtils.h"
#include "Paths.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>


void GlobalSettings::parse_gamedirs(const std::function<void(const QString&)>& callback)
{
    constexpr int LINE_MAX_LEN = 4096;

    for (QString& path : paths::configDirs()) {
        path += QStringLiteral("/game_dirs.txt");

        QFile config_file(path);
        if (!config_file.open(QFile::ReadOnly | QFile::Text))
            continue;

        QTextStream stream(&config_file);
        QString line;
        while (stream.readLineInto(&line, LINE_MAX_LEN)) {
            if (!line.startsWith('#'))
                callback(line);
        }
    }
}
