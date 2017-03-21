#include "Utils.h"

#include <QFileInfo>
#include <QString>


bool validFile(const QString& path) {
    QFileInfo file(path);
    return file.exists() && file.isFile();
}
