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


#include "ApiMeta.h"

#include <QDebug>
#include <QRegularExpression>
#include <QStandardPaths>


namespace ApiParts {

const QString Meta::m_git_revision(QStringLiteral(GIT_REVISION));

Meta::Meta(QObject* parent)
    : QObject(parent)
    , m_loading(false)
    , m_scanning(true)
    , m_scanning_time_ms(0)
{
    using regex = QRegularExpression;
    using qsp = QStandardPaths;

    Q_ASSERT(qsp::standardLocations(qsp::AppConfigLocation).length() > 0);
    m_log_path = qsp::standardLocations(qsp::AppConfigLocation).first();
    m_log_path = m_log_path.replace(regex("(/pegasus-frontend){2}$"), "/pegasus-frontend");
    m_log_path += "/lastrun.log";
}

void Meta::onScanStarted()
{
    m_scanning = true;
}

void Meta::onScanCompleted(qint64 elapsedTime)
{
    m_scanning_time_ms = elapsedTime;
    m_scanning = false;
    qInfo().noquote() << tr("Data files loaded in %1ms").arg(elapsedTime);
}

void Meta::onLoadingCompleted()
{
    m_loading = false;
}

} // namespace ApiParts
