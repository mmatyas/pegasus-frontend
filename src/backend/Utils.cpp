#include "Utils.h"

#include <QFileInfo>
#include <QString>
#include <QVector>


bool validFile(const QString& path) {
    QFileInfo file(path);
    return file.exists() && file.isFile();
}

void parseStoreInt(const QString &str, int &val) {
    bool success = false;
    const int parsed_val = str.toInt(&success);
    if (success)
        val = parsed_val;
}
