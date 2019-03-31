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


#pragma once

#include <QAbstractListModel>
#include <QDir>


struct FolderListEntry {
    const QString name;
    const bool is_dir;

    FolderListEntry(QString name, bool is_dir);

    FolderListEntry(const FolderListEntry&) = delete;
    FolderListEntry& operator=(const FolderListEntry&) = delete;
    FolderListEntry(FolderListEntry&&) = default;
    FolderListEntry& operator=(FolderListEntry&&) = default;
};


class FolderListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString folder READ folder NOTIFY folderChanged)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)

public:
    explicit FolderListModel(QObject* parent = nullptr);

    enum Roles {
        EntryName = Qt::UserRole + 1,
        EntryIsDir,
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override { return m_role_names; }

    Q_INVOKABLE void cd(const QString&);

    QString folder() const { return m_dir_path; }
    const QStringList& nameFilters() const { return m_name_filters; }
    void setNameFilters(QStringList);

signals:
    void folderChanged();

private:
    QDir m_dir;
    QString m_dir_path;
    std::vector<FolderListEntry> m_files;
    QStringList m_name_filters;

    const std::vector<QString> m_drives_cache;
    const QHash<int, QByteArray> m_role_names;
};
