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
    , m_current_game_idx(-1)
    , m_current_platform(nullptr)
    , m_current_game(nullptr)
    , m_init_in_progress(true)
{
    QFuture<void> future = QtConcurrent::run([this]{
        QElapsedTimer timer;
        timer.start();

        this->m_platforms = DataFinder::find();

        this->m_loading_time_ms = timer.elapsed();
    });

    m_loading_watcher.setFuture(future);
    connect(&m_loading_watcher, &QFutureWatcher<void>::finished,
            this, &ApiObject::onLoadingFinished);
}

void ApiObject::onLoadingFinished()
{
    emit platformModelChanged();
    qInfo().noquote() << tr("Data files loaded in %1ms").arg(m_loading_time_ms);

    if (!m_platforms.isEmpty())
        setCurrentPlatformIndex(0);

    m_init_in_progress = false;
    emit initComplete();
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

    resetGameIndex();
}

void ApiObject::resetGameIndex()
{
    // these values are always in pair
    Q_ASSERT((m_current_game_idx == -1) == (m_current_game == nullptr));
    if (!m_current_game) // already reset
        return;

    m_current_game_idx = -1;
    m_current_game = nullptr;
    emit currentGameChanged();
}

void ApiObject::setCurrentPlatformIndex(int idx)
{
    if (idx == m_current_platform_idx)
        return;

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

    if (m_platforms.at(idx)->m_games.isEmpty())
        resetGameIndex();
    else
        setCurrentGameIndex(0);
}

void ApiObject::setCurrentGameIndex(int idx)
{
    if (idx == -1) {
        resetGameIndex();
        return;
    }

    if (!m_current_platform) {
        qWarning() << tr("Could not set game index, the current platform is undefined!");
        return;
    }

    const bool valid_idx = (0 <= idx || idx < m_current_platform->m_games.count());
    if (!valid_idx) {
        qWarning() << tr("Invalid game index #%1").arg(idx);
        return;
    }

    Model::Game* new_game = m_current_platform->m_games.at(idx);
    if (m_current_game == new_game)
        return;

    m_current_game_idx = idx;
    m_current_game = new_game;
    Q_ASSERT(m_current_game);
    emit currentGameChanged();
}

void ApiObject::launchGame()
{
    if (!m_current_platform) {
        qWarning() << tr("The current platform is undefined, you can't launch any games!");
        return;
    }
    if (!m_current_game) {
        qWarning() << tr("The current game is undefined, you can't launch it!");
        return;
    }

    emit prepareLaunch();
}

void ApiObject::onReadyToLaunch()
{
    Q_ASSERT(m_current_platform);
    Q_ASSERT(m_current_game);

    emit executeLaunch(m_current_platform, m_current_game);
}

void ApiObject::onGameFinished()
{
    // TODO: this is where play count could be increased

    emit restoreAfterGame(this);
}
