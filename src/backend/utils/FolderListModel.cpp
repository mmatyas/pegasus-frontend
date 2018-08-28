#include "FolderListModel.h"

#include <QDebug>


namespace {
void remove_if(QFileInfoList& list, const std::function<bool(const QFileInfo&)>& predicate)
{
    const auto start_it = std::remove_if(list.begin(), list.end(), predicate);
    list.erase(start_it, list.end());
}
} // namespace


FolderListEntry::FolderListEntry(QString name, bool is_dir)
    : name(std::move(name))
    , is_dir(is_dir)
{}


FolderListModel::FolderListModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_dir(QDir::root())
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

    const bool success = m_dir.cd(dirName);
    if (success) {
        m_dir_path = m_dir.absolutePath();

        auto filist = m_dir.entryInfoList();
        remove_if(filist, [this](const QFileInfo& fi){
            return !fi.isDir() && !m_name_filters.contains(fi.fileName());
        });

        // adding dotdot manually to avoid getting stuck in the file system
        m_files.emplace_back(QStringLiteral(".."), true);

        for (const auto& fi : qAsConst(filist))
            m_files.emplace_back(fi.fileName(), fi.isDir());
    }
    else {
        m_dir_path = QStringLiteral("/");

        const auto entries = QDir::drives();
        for (const auto& entry : entries)
            m_files.emplace_back(entry.absolutePath(), entry.isDir());
    }

    endResetModel();
    emit folderChanged();
}

void FolderListModel::setNameFilters(QStringList nameFilters)
{
    m_name_filters = std::move(nameFilters);
    cd(QStringLiteral("."));
}
