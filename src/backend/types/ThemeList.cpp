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


#include "ThemeList.h"

#include "ListPropertyFn.h"
#include "Utils.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QUrl>


namespace {

const QString SETTINGSKEY_THEME(QStringLiteral("theme"));

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

} // namespace


namespace Types {

ThemeList::ThemeList(QObject* parent)
    : QObject(parent)
    , m_theme_idx(-1)
{
    findAvailableThemes();
    selectPreferredTheme();
    printChangeMsg();
}

void ThemeList::findAvailableThemes()
{
    constexpr auto filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto flags = QDirIterator::FollowSymlinks;

    const QString ini_filename(QStringLiteral("theme.ini"));
    const QString qml_filename(QStringLiteral("theme.qml"));
    const QString warn_missingfile = QObject::tr("Warning: no `%1` file found in `%2`, theme skipped");
    const QString warn_missingentry = QObject::tr("Warning: there is no `%1` entry in `%2`, theme skipped");

    const QString INIKEY_NAME(QStringLiteral("name"));
    const QString INIKEY_AUTHOR(QStringLiteral("author"));
    const QString INIKEY_VERSION(QStringLiteral("version"));
    const QString INIKEY_SUMMARY(QStringLiteral("summary"));
    const QString INIKEY_DESC(QStringLiteral("description"));

    QStringList search_paths = themeDirectories();
    for (auto& path : search_paths) {
        QDirIterator themedir(path, filters, flags);
        while (themedir.hasNext()) {
            const QString basedir = themedir.next() % '/';
            const QString ini_path = basedir % ini_filename;
            QString qml_path = basedir % qml_filename;

            if (!validFileQt(ini_path)) {
                qWarning().noquote() << warn_missingfile.arg(ini_filename, basedir);
                continue;
            }
            if (!validFileQt(qml_path)) {
                qWarning().noquote() << warn_missingfile.arg(qml_filename, basedir);
                continue;
            }

            const QSettings metadata(ini_path, QSettings::IniFormat);
            if (!metadata.contains(INIKEY_NAME)) {
                qWarning().noquote() << warn_missingentry.arg(INIKEY_NAME, ini_filename);
                continue;
            }

            // add the qrc/file protocol prefix
            const bool is_builtin = basedir.startsWith(':');
            if (is_builtin)
                qml_path = QLatin1String("qrc://") % qml_path.midRef(1);
            else
                qml_path = QUrl::fromLocalFile(qml_path).toString();

            m_themes.append(new Types::Theme(
                basedir, qml_path,
                metadata.value(INIKEY_NAME).toString(),
                metadata.value(INIKEY_AUTHOR).toString(),
                metadata.value(INIKEY_VERSION).toString(),
                metadata.value(INIKEY_SUMMARY).toString(),
                metadata.value(INIKEY_DESC).toString(),
                this
            ));

            qInfo().noquote() << tr("Found theme '%1' (`%2`)")
                                 .arg(m_themes.last()->name(), m_themes.last()->dir());
        }
    }

    std::sort(m_themes.begin(), m_themes.end(),
        [](const Types::Theme* const a, const Types::Theme* const b) {
            return a->compare(*b) < 0;
        }
    );
}

void ThemeList::selectPreferredTheme()
{
    // this method should be called after all themes have been found
    Q_ASSERT(!m_themes.isEmpty());


    // A. Try to use the saved config value
    const QString requested_theme = QSettings().value(SETTINGSKEY_THEME).toString();
    if (!requested_theme.isEmpty())
        m_theme_idx = indexOfTheme(requested_theme);

    // B. Fall back to the built-in theme
    //    Either the config value is invalid, or has missing files,
    //    thus not present in `m_themes`.
    if (m_theme_idx < 0)
        m_theme_idx = indexOfTheme(":/themes/pegasus-grid/");


    Q_ASSERT(m_theme_idx >= 0 && m_theme_idx < m_themes.length());
}

void ThemeList::printChangeMsg() const
{
    qInfo().noquote() << QObject::tr("Theme set to '%1' (%2)")
                         .arg(current()->name(), current()->dir());
}

Theme* ThemeList::current() const
{
    Q_ASSERT(0 <= index() && index() < m_themes.length());
    return m_themes.at(index());
}

int ThemeList::indexOfTheme(const QString& dir_path) const
{
    for (int idx = 0; idx < m_themes.count(); idx++) {
        if (m_themes.at(idx)->dir() == dir_path)
            return idx;
    }

    return -1;
}

void ThemeList::setIndex(int idx)
{
    // verify
    if (idx == m_theme_idx)
        return;

    const bool valid_idx = (0 <= idx && idx < m_themes.length());
    if (!valid_idx) {
        qWarning() << tr("Invalid theme index #%1").arg(idx);
        return;
    }

    // set
    m_theme_idx = idx;
    printChangeMsg();
    emit themeChanged();

    // remember
    QSettings().setValue(SETTINGSKEY_THEME, m_themes.at(idx)->dir());
}

QQmlListProperty<Theme> ThemeList::getListProp()
{
    static constexpr auto count = &listproperty_count<Theme>;
    static constexpr auto at = &listproperty_at<Theme>;

    return {this, &m_themes, count, at};
}

} // namespace Types
