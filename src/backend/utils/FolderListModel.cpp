// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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


#include "FolderListModel.h"

#include "utils/StdHelpers.h"

#ifdef Q_OS_ANDROID
#include "platform/AndroidHelpers.h"
#include <QStandardPaths>
#endif


namespace {
void remove_if(QFileInfoList& list, const std::function<bool(const QFileInfo&)>& predicate)
{
    const auto start_it = std::remove_if(list.begin(), list.end(), predicate);
    list.erase(start_it, list.end());
}

std::vector<QString> drives()
{
#if defined(Q_OS_ANDROID)
    return android::storage_paths();

#elif defined(Q_OS_UNIX)
    // Fast path for *nix
    return { QStringLiteral("/") };

#else
    std::vector<QString> out;

    const auto drive_files = QDir::drives();
    for (const auto& file : drive_files)
        out.emplace_back(QDir::toNativeSeparators(file.absolutePath()));

    return out;
#endif
}

QDir startup_dir()
{
#ifdef Q_OS_ANDROID
    // Avoid pointing to some internal directory, return the primary shared storage
    return android::primary_storage_path();
#else
    return QDir::home();
#endif
}

bool is_drive_root(const QString& path, const std::vector<QString>& drives)
{
    return VEC_CONTAINS(drives, path);
}
} // namespace


FolderListEntry::FolderListEntry(QString name, bool is_dir)
    : name(std::move(name))
    , is_dir(is_dir)
{}


FolderListModel::FolderListModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_dir(startup_dir())
    , m_drives_cache(drives())
    , m_role_names({
        { EntryName, "name" },
        { EntryIsDir, "isDir" },
    })
{
    m_dir.setSorting(QDir::Name | QDir::DirsFirst | QDir::IgnoreCase);
    m_dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable);
    setNameFilters(m_name_filters);
}

int FolderListModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_files.size());
}

QVariant FolderListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || rowCount() <= index.row())
        return {};

    const auto row = static_cast<size_t>(index.row());
    if (m_files.size() <= row)
        return {};

    const FolderListEntry& entry = m_files.at(row);
    switch (role) {
        case EntryName:
            return entry.name;
        case EntryIsDir:
            return entry.is_dir;
    }

    return {};
}

void FolderListModel::cd(const QString& dirName)
{
    beginResetModel();
    m_files.clear();

    bool goto_root = (dirName == QLatin1String("..")) && is_drive_root(m_dir_path, m_drives_cache);
    if (!goto_root) {
        // try to step into the directory, otherwise fallback to the root
        goto_root = (m_dir.cd(dirName) == false);
        // TODO: update the drives cache here on error
    }

    if (goto_root) {
        m_dir_path = QDir::toNativeSeparators(QStringLiteral("/"));
        for (const QString& drive : m_drives_cache)
            m_files.emplace_back(drive, true);
    }
    else {
        m_dir_path = QDir::toNativeSeparators(m_dir.absolutePath());

        auto filist = m_dir.entryInfoList();
        remove_if(filist, [this](const QFileInfo& fi){
            return !fi.isDir() && !m_name_filters.contains(fi.fileName());
        });

        // adding dotdot manually to avoid getting stuck in the file system
        m_files.emplace_back(QStringLiteral(".."), true);

        for (const auto& fi : qAsConst(filist))
            m_files.emplace_back(fi.fileName(), fi.isDir());
    }

    endResetModel();
    emit folderChanged();
}

void FolderListModel::setNameFilters(QStringList nameFilters)
{
    m_name_filters = std::move(nameFilters);
    emit nameFiltersChanged();
    cd(QStringLiteral("."));
}
