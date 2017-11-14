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


#include "Api.h"

#include "ListPropertyFn.h"


ApiObject::ApiObject(QObject* parent)
    : QObject(parent)
{
    connect(m_settings.localesPtr(), &ApiParts::LocaleSettings::localeChanged,
            this, &ApiObject::localeChanged);
    connect(&m_system, &Api::System::appCloseRequested,
            this, &ApiObject::appCloseRequested);
    connect(&m_filters, &ApiParts::Filters::filtersChanged,
            this, &ApiObject::onFiltersChanged);

    connect(&m_platform_list, &Model::PlatformList::currentChanged,
            this, &ApiObject::currentPlatformChanged);
    connect(&m_platform_list, &Model::PlatformList::currentPlatformGameChanged,
            this, &ApiObject::currentGameChanged);

    connect(&m_platform_list, &Model::PlatformList::scanCompleted,
            this, &ApiObject::onScanCompleted);
    connect(&m_platform_list, &Model::PlatformList::newGamesScanned,
            &m_meta, &ApiParts::Meta::onNewGamesScanned);
}

void ApiObject::startScanning()
{
    m_meta.onScanStarted();
    m_platform_list.startScanning();
}

void ApiObject::onScanCompleted()
{
    m_meta.onScanCompleted(m_platform_list.scanDuration());
    m_meta.onLoadingCompleted();
}

void ApiObject::launchGame()
{
    if (!currentPlatform()) {
        qWarning() << tr("The current platform is undefined, you can't launch any games!");
        return;
    }
    if (!currentGame()) {
        qWarning() << tr("The current game is undefined, you can't launch it!");
        return;
    }

    emit prepareLaunch();
}

void ApiObject::onReadyToLaunch()
{
    Q_ASSERT(currentPlatform());
    Q_ASSERT(currentGame());
    emit executeLaunch(currentPlatform(), currentGame());
}

void ApiObject::onGameFinished()
{
    // TODO: this is where play count could be increased
    emit restoreAfterGame(this);
}

void ApiObject::onFiltersChanged()
{
    for (Model::Platform* const platform : m_platform_list.model())
        platform->gameListMut().applyFilters(m_filters);
}
