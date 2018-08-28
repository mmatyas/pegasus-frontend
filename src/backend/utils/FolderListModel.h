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

    const QHash<int, QByteArray> m_role_names;
};
