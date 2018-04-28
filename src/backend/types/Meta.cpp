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


namespace Types {

const QString Meta::m_git_revision(QStringLiteral(GIT_REVISION));

Meta::Meta(QObject* parent)
    : QObject(parent)
    , m_log_path(paths::writableConfigDir() + QStringLiteral("/lastrun.log"))
    , m_loading(true)
    , m_scanning(false)
    , m_scanning_meta(false)
    , m_scanning_time_ms(0)
    , m_game_count(0)
{
}

void Meta::clearQMLCache()
{
    emit qmlClearCacheRequested();
}

void Meta::onScanStarted()
{
    m_scanning = true;
    emit scanningChanged();
}

void Meta::onScanMetaStarted()
{
    m_scanning_meta = true;
    emit scanningMetaChanged();
}

void Meta::onScanCompleted(qint64 elapsedTime)
{
    m_scanning_time_ms = elapsedTime;
    m_scanning = false;
    m_scanning_meta = false;
    emit scanningMetaChanged();
    emit scanningChanged();

    qInfo().noquote() << tr_log("Data files loaded in %1ms").arg(elapsedTime);
}

void Meta::onLoadingCompleted()
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

} // namespace Types
