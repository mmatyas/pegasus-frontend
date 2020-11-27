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


#include "Themes.h"

#include "AppSettings.h"
#include "Log.h"
#include "Paths.h"
#include "parsers/MetaFile.h"
#include "utils/HashMap.h"
#include "utils/PathCheck.h"

#include <QDirIterator>
#include <QStringBuilder>
#include <QUrl>


namespace {
QStringList theme_directories()
{
    QStringList theme_dirs = paths::configDirs();
    for (QString& path : theme_dirs)
        path += QLatin1String("/themes/");

    return theme_dirs;
}

// TODO: This could be optimized further, like the Pegasus provider code
HashMap<QString, QString> read_metafile(const QString& config_file_path)
{
    HashMap<QString, QString> result;

    metafile::read_file(
        config_file_path,
        [&](const metafile::Entry& entry){
            result.emplace(entry.key, metafile::merge_lines(entry.values));
        },
        [&](const metafile::Error& error){
            Log::warning(LOGMSG("`%1`, line %2: %3")
                .arg(config_file_path, QString::number(error.line), error.message));
        });
    return result;
}

std::vector<model::ThemeEntry> find_available_themes()
{
    constexpr auto filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto flags = QDirIterator::FollowSymlinks;

    const auto META_FILENAME(QStringLiteral("theme.cfg"));
    const auto QML_FILENAME(QStringLiteral("theme.qml"));

    const auto META_KEY_NAME(QStringLiteral("name"));
    const auto META_KEY_AUTHOR(QStringLiteral("author"));
    const auto META_KEY_VERSION(QStringLiteral("version"));
    const auto META_KEY_SUMMARY(QStringLiteral("summary"));
    const auto META_KEY_DESC(QStringLiteral("description"));
    //const QString META_KEY_KEYWORDS(QStringLiteral("keywords"));
    //const QString META_KEY_HOMEPAGE(QStringLiteral("homepage"));

    const auto E_FILE_MISSING = LOGMSG("No `%1` file found in `%2`, theme skipped");
    const auto E_KEY_MISSING = LOGMSG("There is no `%1` entry in `%2`, theme skipped");


    const QStringList search_paths = theme_directories();

    std::vector<model::ThemeEntry> themes;
    themes.reserve(static_cast<size_t>(search_paths.count()));

    for (auto& path : search_paths) {
        QDirIterator themedir(path, filters, flags);
        while (themedir.hasNext()) {
            const QString basedir = themedir.next() % '/';
            const QString meta_path = basedir % META_FILENAME;
            QString qml_path = basedir % QML_FILENAME;

            if (!::validFile(meta_path)) {
                Log::warning(E_FILE_MISSING.arg(META_FILENAME, basedir));
                continue;
            }
            if (!::validFile(qml_path)) {
                Log::warning(E_FILE_MISSING.arg(QML_FILENAME, basedir));
                continue;
            }

            HashMap<QString, QString> metadata = read_metafile(meta_path);
            if (!metadata.count(META_KEY_NAME)) {
                Log::warning(E_KEY_MISSING.arg(META_KEY_NAME, meta_path));
                continue;
            }

            // add the qrc/file protocol prefix
            const bool is_builtin = basedir.startsWith(':');
            qml_path = is_builtin
                ? QLatin1String("qrc://") % qml_path.midRef(1)
                : QUrl::fromLocalFile(qml_path).toString();

            themes.emplace_back(
                basedir,
                qml_path,
                metadata[META_KEY_NAME],
                metadata[META_KEY_AUTHOR],
                metadata[META_KEY_VERSION],
                metadata[META_KEY_SUMMARY],
                metadata[META_KEY_DESC]);

            Log::info(LOGMSG("Found theme `%1` at `%2`")
                .arg(themes.back().name, themes.back().root_dir));
        }
    }

    std::sort(themes.begin(), themes.end(),
        [](const model::ThemeEntry& a, const model::ThemeEntry& b) {
            return QString::localeAwareCompare(a.name, b.name) < 0;
        }
    );

    return themes;
}
} // namespace


namespace model {
ThemeEntry::ThemeEntry(QString root_dir, QString root_qml, QString name,
                       QString author, QString version, QString summary, QString description)
    : root_dir(std::move(root_dir))
    , root_qml(std::move(root_qml))
    , name(std::move(name))
    , author(std::move(author))
    , version(std::move(version))
    , summary(std::move(summary))
    , description(std::move(description))
{}


Themes::Themes(QObject* parent)
    : QAbstractListModel(parent)
    , m_role_names({
        { Roles::Name, QByteArrayLiteral("name") },
        { Roles::Author, QByteArrayLiteral("author") },
        { Roles::Version, QByteArrayLiteral("version") },
        { Roles::Summary, QByteArrayLiteral("summary") },
        { Roles::Description, QByteArrayLiteral("description") },
    })
    , m_themes(find_available_themes())
{
    select_preferred_theme();
    print_change();
}

void Themes::select_preferred_theme()
{
    // A. Try to use the saved config value
    if (select_theme(AppSettings::general.theme))
        return;

    // B. Fall back to the built-in theme
    if (select_theme(AppSettings::general.DEFAULT_THEME))
        return;

    Q_UNREACHABLE();
}

bool Themes::select_theme(const QString& root_dir)
{
    if (root_dir.isEmpty())
        return false;

    const QFileInfo root_finfo(paths::writableConfigDir(), root_dir);

    for (size_t idx = 0; idx < m_themes.size(); idx++) {
        if (QFileInfo(m_themes.at(idx).root_dir) == root_finfo) {
            m_current_idx = idx;
            return true;
        }
    }

    Log::warning(LOGMSG("Requested theme `%1` not found, falling back to default")
        .arg(root_finfo.absoluteFilePath()));
    return false;
}

void Themes::print_change() const
{
    const auto& current = m_themes.at(m_current_idx);

    Log::info(LOGMSG("Theme set to `%1` (`%2`)")
        .arg(current.name, current.root_dir));
}

int Themes::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_themes.size());
}

QVariant Themes::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || rowCount() <= index.row())
        return {};

    const auto& theme = m_themes.at(static_cast<size_t>(index.row()));
    switch (role) {
        case Roles::Name:
            return theme.name;
        case Roles::Author:
            return theme.author;
        case Roles::Version:
            return theme.version;
        case Roles::Summary:
            return theme.summary;
        case Roles::Description:
            return theme.description;
        default:
            return {};
    }
}

void Themes::setCurrentIndex(int idx_int)
{
    const auto idx = static_cast<size_t>(idx_int);

    // verify
    if (idx == m_current_idx)
        return;

    if (m_themes.size() <= idx) {
        Log::warning(LOGMSG("Invalid theme index #%1").arg(idx));
        return;
    }

    // set
    m_current_idx = idx;
    print_change();
    emit themeChanged();

    // remember
    AppSettings::general.theme = m_themes.at(m_current_idx).root_dir;
    AppSettings::save_config();
}
} // namespace model
