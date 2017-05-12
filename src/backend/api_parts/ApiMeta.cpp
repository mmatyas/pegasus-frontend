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


namespace ApiParts {

Meta::Meta(QObject *parent)
    : QObject(parent)
    , m_git_revision(GIT_REVISION)
    , m_loading(true)
    , m_loading_time_ms(0)
{
}

void Meta::setElapsedLoadingTime(qint64 ms)
{
    m_loading_time_ms = ms;
    qInfo().noquote() << tr("Data files loaded in %1ms").arg(ms);
}

void Meta::onApiLoadingFinished()
{
    m_loading = false;
    emit loadingComplete();
}

} // namespace ApiParts
