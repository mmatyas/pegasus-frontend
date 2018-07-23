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


#include "Meta.h"

#include "LocaleUtils.h"
#include "Paths.h"

#include <QDebug>


namespace model {

const QString Meta::m_git_revision(QStringLiteral(GIT_REVISION));

Meta::Meta(QObject* parent)
    : QObject(parent)
    , m_log_path(paths::writableConfigDir() + QStringLiteral("/lastrun.log"))
    , m_loading(true)
    , m_loading_progress(0.f)
    , m_game_count(0)
{
}

void Meta::clearQMLCache()
{
    emit qmlClearCacheRequested();
}

void Meta::onFirstPhaseCompleted(qint64 elapsedTime)
{
    qInfo().noquote() << tr_log("Games found in %1ms").arg(elapsedTime);

    m_loading_progress = 0.5f;
    emit loadingProgressChanged();
}

void Meta::onSecondPhaseCompleted(qint64 elapsedTime)
{
    qInfo().noquote() << tr_log("Assets and metadata found in %1ms").arg(elapsedTime);

    m_loading_progress = 1.0f;
    emit loadingProgressChanged();
}

void Meta::onUiReady()
{
    m_loading = false;
    emit loadingChanged();
}

void Meta::onGameCountUpdate(int game_count)
{
    if (m_game_count != game_count) {
        m_game_count = game_count;
        emit gameCountChanged();
    }
}

} // namespace model
