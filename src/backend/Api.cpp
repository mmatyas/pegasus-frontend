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

#include "DataFinder.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QRegularExpression>
#include <QtConcurrent/QtConcurrent>


ApiObject::ApiObject(QObject* parent)
    : QObject(parent)
    , m_current_platform_idx(-1)
    , m_current_platform(nullptr)
{
    // launch the game search on a parallel thread
    QFuture<void> future = QtConcurrent::run([this]{
        QElapsedTimer timer;
        timer.start();

        this->m_platforms = DataFinder::find();
        this->m_meta.setElapsedLoadingTime(timer.elapsed());

        // set the correct thread for the QObjects
        for (Model::Platform* const platform : qAsConst(m_platforms))
            platform->moveToThread(this->thread());
    });

    m_loading_watcher.setFuture(future);
    connect(&m_loading_watcher, &QFutureWatcher<void>::finished,
            this, &ApiObject::onLoadingFinished);


    // subcomponent signals
    connect(&m_settings, &ApiParts::Settings::languageChanged,
            this, &ApiObject::languageChanged);
    connect(&m_filters, &ApiParts::Filters::filtersChanged,
            this, &ApiObject::onFiltersChanged);
}

void ApiObject::onLoadingFinished()
{
    // NOTE: `tr` (see below) uses `int`; assuming we have
    //       less than 2 million games, it will be enough
    int32_t game_count = 0;

    for (int i = 0; i < m_platforms.length(); i++) {
        Model::Platform* const platform_ptr = m_platforms.at(i);
        Q_ASSERT(platform_ptr);

        connect(platform_ptr, &Model::Platform::currentGameChanged,
                [this, i](){ ApiObject::onPlatformGameChanged(i); });

        Model::Platform& platform = *platform_ptr;
        platform.lockGameList();
        game_count += platform.allGames().count();
    }
    qInfo().noquote() << tr("%n games found", "", game_count);

    emit platformModelChanged();

    if (!m_platforms.isEmpty())
        setCurrentPlatformIndex(0);

    m_meta.onApiLoadingFinished();
}

QQmlListProperty<Model::Platform> ApiObject::getPlatformsProp()
{
   return QQmlListProperty<Model::Platform>(this, m_platforms);
}

void ApiObject::resetPlatformIndex()
{
    // these values are always in pair
    Q_ASSERT((m_current_platform_idx == -1) == (m_current_platform == nullptr));
    if (!m_current_platform) // already reset
        return;

    m_current_platform_idx = -1;
    m_current_platform = nullptr;
    emit currentPlatformChanged();

    // TODO: this is unnecessary
    for (Model::Platform* platform : qAsConst(m_platforms))
        platform->resetGameIndex();
}

void ApiObject::setCurrentPlatformIndex(int idx)
{
    if (idx == m_current_platform_idx)
        return;

    // TODO: drop -1 support
    if (idx == -1) {
        resetPlatformIndex();
        return;
    }

    const bool valid_idx = (0 <= idx || idx < m_platforms.count());
    if (!valid_idx) {
        qWarning() << tr("Invalid platform index #%1").arg(idx);
        return;
    }

    m_current_platform_idx = idx;
    m_current_platform = m_platforms.at(idx);
    Q_ASSERT(m_current_platform);

    emit currentPlatformChanged();
    emit currentGameChanged();
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

void ApiObject::onPlatformGameChanged(int platformIndex)
{
    if (platformIndex == m_current_platform_idx)
        emit currentGameChanged();
}

void ApiObject::onFiltersChanged()
{
    // TODO: use QtConcurrent::blockingMap here
    for (Model::Platform* const platform : qAsConst(m_platforms))
        platform->applyFilters(m_filters);
}
