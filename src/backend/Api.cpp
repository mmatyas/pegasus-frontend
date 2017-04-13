#include "Api.h"

#include "Es2XmlReader.h"

#include <QDebug>


ApiObject::ApiObject(QObject* parent)
    : QObject(parent)
    , m_current_platform_idx(-1)
    , m_current_game_idx(-1)
    , m_current_platform(nullptr)
    , m_current_game(nullptr)
{
    Es2XmlReader::read(m_platforms); // TODO: check result
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

}
