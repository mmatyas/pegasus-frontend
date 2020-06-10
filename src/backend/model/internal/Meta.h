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

#include "CliArgs.h"

#include <QObject>


namespace model {

/// Provides information about the program for the frontend layer
class Meta : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(float loadingProgress READ loadingProgress NOTIFY loadingProgressChanged)

    Q_PROPERTY(int gameCount READ gameCount NOTIFY gameCountChanged)

    Q_PROPERTY(QString gitRevision MEMBER m_git_revision CONSTANT)
    Q_PROPERTY(QString gitDate MEMBER m_git_date CONSTANT)
    Q_PROPERTY(QString logFilePath MEMBER m_log_path CONSTANT)

    Q_PROPERTY(bool allowReboot MEMBER m_enable_menu_reboot CONSTANT)
    Q_PROPERTY(bool allowShutdown MEMBER m_enable_menu_shutdown CONSTANT)
    Q_PROPERTY(bool allowAppClose MEMBER m_enable_menu_appclose CONSTANT)

public:
    explicit Meta(const backend::CliArgs& args, QObject* parent = nullptr);

    void startLoading();
    void onUiReady();

public:
    Q_INVOKABLE void resetLoadingState();
    Q_INVOKABLE void clearQMLCache();

    bool isLoading() const { return m_loading; }
    float loadingProgress() const { return m_loading_progress; }

    int gameCount() const { return m_game_count; }

public slots:
    void onFirstPhaseCompleted(qint64 elapsedTime);
    void onSecondPhaseCompleted(qint64 elapsedTime);

    void onGameCountUpdate(int game_count);

signals:
    void loadingChanged();
    void loadingProgressChanged();
    void gameCountChanged();

    void qmlClearCacheRequested();

private:
    static const QString m_git_revision;
    static const QString m_git_date;
    const QString m_log_path;

    const bool m_enable_menu_reboot;
    const bool m_enable_menu_shutdown;
    const bool m_enable_menu_appclose;

    bool m_loading;
    float m_loading_progress;

    int m_game_count;
};

} // namespace model
