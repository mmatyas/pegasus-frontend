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


#include "AppFiles.h"

#include "Utils.h"
#include "model/Locale.h"
#include "model/ThemeEntry.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>


namespace {

QStringList themeDirectories()
{
    QStringList theme_dirs;
    theme_dirs << ":";
    theme_dirs << QCoreApplication::applicationDirPath();
#ifdef INSTALL_DATADIR
    if (validPath(INSTALL_DATADIR))
        theme_dirs << QString(INSTALL_DATADIR);
#endif
    theme_dirs << QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    theme_dirs << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    for (QString& path : theme_dirs) {
        path += "/themes/";
        // do not add the organization name to the search path
        path.replace(QLatin1String("/pegasus-frontend/pegasus-frontend/"),
                     QLatin1String("/pegasus-frontend/"));
    }

    return theme_dirs;
}

// because the theme files may be embedded,
// we can't use platform-specific shortcuts
bool fileExists(const QString& path)
{
    QFileInfo file(path);
    return file.exists() && file.isFile();
}

} // namespace


namespace model_providers {

const QLatin1String AppFiles::DEFAULT_LOCALE_TAG("en-DK");

std::vector<Model::Locale*> AppFiles::findAvailableLocales()
{
    const int QM_PREFIX_LEN = 8; // length of "pegasus_"
    const int QM_SUFFIX_LEN = 3; // length of ".qm"
    const QString DEFAULT_FILENAME("pegasus_" + DEFAULT_LOCALE_TAG + ".qm");

    // find the available languages
    QStringList qm_files = QDir(":/lang").entryList(QStringList("*.qm"));
    qm_files.append(DEFAULT_FILENAME); // default placeholder
    qm_files.sort();

    std::vector<Model::Locale*> output;
    for (const QString& filename : qAsConst(qm_files)) {
        const int locale_tag_len = filename.length() - QM_PREFIX_LEN - QM_SUFFIX_LEN;
        Q_ASSERT(locale_tag_len > 0);

        const QString locale_tag = filename.mid(QM_PREFIX_LEN, locale_tag_len);
        output.emplace_back(locale_tag);
    }
    return output;
}

std::vector<Model::Theme*> AppFiles::findAvailableThemes()
{
    const auto filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    const auto flags = QDirIterator::FollowSymlinks;

    const QString ini_filename("theme.ini");
    const QString qml_filename("theme.qml");
    const QString warn_missingfile = QObject::tr("Warning: no `%1` file found in `%2`, theme skipped");
    const QString warn_missingentry = QObject::tr("Warning: there is no `%1` entry in `%2`, theme skipped");

    const QString INIKEY_NAME("name");
    const QString INIKEY_AUTHOR("author");
    const QString INIKEY_VERSION("version");
    const QString INIKEY_SUMMARY("summary");
    const QString INIKEY_DESC("description");

    std::vector<Model::Theme*> output;

    QStringList search_paths = themeDirectories();
    for (auto& path : search_paths) {
        QDirIterator themedir(path, filters, flags);
        while (themedir.hasNext()) {
            const auto basedir = themedir.next() + '/';
            const auto ini_path = basedir + ini_filename;
            const auto qml_path = basedir + qml_filename;

            if (!fileExists(ini_path)) {
                qWarning().noquote() << warn_missingfile.arg(ini_filename, basedir);
                continue;
            }
            if (!fileExists(qml_path)) {
                qWarning().noquote() << warn_missingfile.arg(qml_filename, basedir);
                continue;
            }

            const QSettings metadata(ini_path, QSettings::IniFormat);
            if (!metadata.contains(INIKEY_NAME)) {
                qWarning().noquote() << warn_missingentry.arg(INIKEY_NAME, ini_filename);
                continue;
            }

            output.emplace_back(
                basedir, qml_path,
                metadata.value(INIKEY_NAME).toString(),
                metadata.value(INIKEY_AUTHOR).toString(),
                metadata.value(INIKEY_VERSION).toString(),
                metadata.value(INIKEY_SUMMARY).toString(),
                metadata.value(INIKEY_DESC).toString()
            );
        }
    }

    std::sort(output.begin(), output.end(),
        [](const Model::Theme* a, const Model::Theme* b) {
            return a->compare(*b) < 0;
        }
    );

    return output;
}

} // namespace model_providers
