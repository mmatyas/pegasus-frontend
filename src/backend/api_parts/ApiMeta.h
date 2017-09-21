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


#pragma once

#include <QObject>


namespace ApiParts {

/// Provides information about the program for the frontend layer
class Meta : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(QString gitRevision MEMBER m_git_revision CONSTANT)
    Q_PROPERTY(QString logFilePath MEMBER m_log_path CONSTANT)

public:
    explicit Meta(QObject* parent = nullptr);

    bool isLoading() const { return m_loading; }
    bool isScanning() const { return m_scanning; }

public slots:
    void onScanStarted();
    void onScanCompleted(qint64 elapsedTime);
    void onLoadingCompleted();

signals:
    void loadingChanged();
    void scanningChanged();

private:
    static const QString m_git_revision;
    QString m_log_path;

    bool m_loading;
    bool m_scanning;
    qint64 m_scanning_time_ms;
};

} // namespace ApiParts
