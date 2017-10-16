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

#include "PropertyListMacro.h"


ApiObject::ApiObject(QObject* parent)
    : QObject(parent)
{
    connect(m_settings.localesPtr(), &ApiParts::LocaleSettings::localeChanged,
            this, &ApiObject::localeChanged);
    connect(&m_system, &ApiParts::System::appCloseRequested,
            this, &ApiObject::appCloseRequested);
    connect(&m_filters, &ApiParts::Filters::filtersChanged,
            [this]{ m_platforms.onFiltersChanged(m_filters); });

    connect(&m_platforms, &ApiParts::Platforms::modelChanged,
            this, &ApiObject::platformModelChanged);
    connect(&m_platforms, &ApiParts::Platforms::indexChanged,
            this, &ApiObject::currentPlatformIndexChanged);
    connect(&m_platforms, &ApiParts::Platforms::platformChanged,
            this, &ApiObject::currentPlatformChanged);
    connect(&m_platforms, &ApiParts::Platforms::platformGameChanged,
            this, &ApiObject::currentGameChanged);
    connect(&m_platforms, &ApiParts::Platforms::scanCompleted,
            this, &ApiObject::onScanCompleted);
}

void ApiObject::startScanning()
{
    m_meta.onScanStarted();
    m_platforms.startScanning();
}

void ApiObject::onScanCompleted()
{
    m_meta.onScanCompleted(m_platforms.scanDuration());
    m_meta.onLoadingCompleted();
}

QQmlListProperty<Model::Platform> ApiObject::getPlatformsProp()
{
    PROPERTYLIST_GETTER_SPEC(m_platforms.allPlatforms(), Model::Platform)
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
