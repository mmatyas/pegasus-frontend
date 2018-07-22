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

#include "utils/HashMap.h"

#include "ConfigFile.h"
#include "ListPropertyFn.h"
#include "LocaleUtils.h"
#include "Paths.h"
#include "Utils.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include <QStringBuilder>
#include <QUrl>


namespace {

const QString SETTINGSKEY_THEME(QStringLiteral("theme"));

QStringList themeDirectories()
{
    QStringList theme_dirs = paths::configDirs();
    for (QString& path : theme_dirs)
        path += "/themes/";

    return theme_dirs;
}

HashMap<QString, QString> read_metafile(const QString& config_file_path)
{
    HashMap<QString, QString> result;

    config::readFile(
        config_file_path,
        [&](const int, const QString key, const QString val){
            if (result.count(key))
                result[key] += QStringLiteral(", ") % val;
            else
                result.emplace(key, val);
        },
        [&](const int linenum, const QString msg){
            qWarning().noquote() << tr_log("`%1`, line %2: %3")
                .arg(config_file_path, QString::number(linenum), msg);
        });
    return result;
}

} // namespace


namespace model {

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

    const QString meta_filename(QStringLiteral("theme.cfg"));
    const QString qml_filename(QStringLiteral("theme.qml"));
    const QString warn_missingfile = tr_log("[warning] No `%1` file found in `%2`, theme skipped");
    const QString warn_missingentry = tr_log("[warning] There is no `%1` entry in `%2`, theme skipped");

    const QString META_KEY_NAME(QStringLiteral("name"));
    const QString META_KEY_AUTHOR(QStringLiteral("author"));
    const QString META_KEY_VERSION(QStringLiteral("version"));
    const QString META_KEY_SUMMARY(QStringLiteral("summary"));
    const QString META_KEY_DESC(QStringLiteral("description"));
    //const QString META_KEY_KEYWORDS(QStringLiteral("keywords"));
    //const QString META_KEY_HOMEPAGE(QStringLiteral("homepage"));

    QStringList search_paths = themeDirectories();
    for (auto& path : search_paths) {
        QDirIterator themedir(path, filters, flags);
        while (themedir.hasNext()) {
            const QString basedir = themedir.next() % '/';
            const QString meta_path = basedir % meta_filename;
            QString qml_path = basedir % qml_filename;

            if (!::validFile(meta_path)) {
                qWarning().noquote() << warn_missingfile.arg(meta_filename, basedir);
                continue;
            }
            if (!::validFile(qml_path)) {
                qWarning().noquote() << warn_missingfile.arg(qml_filename, basedir);
                continue;
            }

            HashMap<QString, QString> metadata = read_metafile(meta_path);
            if (!metadata.count(META_KEY_NAME)) {
                qWarning().noquote() << warn_missingentry.arg(META_KEY_NAME, meta_path);
                continue;
            }

            // add the qrc/file protocol prefix
            const bool is_builtin = basedir.startsWith(':');
            if (is_builtin)
                qml_path = QLatin1String("qrc://") % qml_path.midRef(1);
            else
                qml_path = QUrl::fromLocalFile(qml_path).toString();

            m_themes.append(new model::Theme(
                basedir, qml_path,
                metadata[META_KEY_NAME],
                metadata[META_KEY_AUTHOR],
                metadata[META_KEY_VERSION],
                metadata[META_KEY_SUMMARY],
                metadata[META_KEY_DESC],
                this
            ));

            qInfo().noquote() << tr_log("Found theme '%1' (`%2`)")
                                 .arg(m_themes.last()->name(), m_themes.last()->dir());
        }
    }

    std::sort(m_themes.begin(), m_themes.end(),
        [](const model::Theme* const a, const model::Theme* const b) {
            return a->compare(*b) < 0;
        }
    );
}

void ThemeList::selectPreferredTheme()
{
    // this method should be called after all themes have been found
    Q_ASSERT(!m_themes.isEmpty());


    // A. Try to use the saved config value
    const QString requested_theme = QSettings(paths::configIniPath(), QSettings::IniFormat)
                                    .value(SETTINGSKEY_THEME).toString();
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
    qInfo().noquote() << tr_log("Theme set to '%1' (`%2`)")
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
        qWarning() << tr_log("Invalid theme index #%1").arg(idx);
        return;
    }

    // set
    m_theme_idx = idx;
    printChangeMsg();
    emit themeChanged();

    // remember
    QSettings settings(paths::configIniPath(), QSettings::IniFormat);
    settings.setValue(SETTINGSKEY_THEME, m_themes.at(idx)->dir());
}

QQmlListProperty<Theme> ThemeList::getListProp()
{
    static constexpr auto count = &listproperty_count<Theme>;
    static constexpr auto at = &listproperty_at<Theme>;

    return {this, &m_themes, count, at};
}

} // namespace model
