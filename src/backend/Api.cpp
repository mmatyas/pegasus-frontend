#include "Api.h"

#include "Es2XmlReader.h"

#include <QDebug>


ApiObject::ApiObject(QObject* parent)
    : QObject(parent)
    , m_current_platform_idx(-1)
    , m_current_game_idx(-1)
{
    Es2XmlReader::read(m_platforms); // TODO: check result
}

QQmlListProperty<Model::Platform> ApiObject::getPlatformsProp()
{
    return QQmlListProperty<Model::Platform>(this, m_platforms);
}

void ApiObject::setCurrentPlatformIndex(int idx)
{
    if (idx == m_current_platform_idx ||
        idx < 0 || idx >= m_platforms.count())
        return;

    m_current_platform_idx = idx;
    m_current_game_idx = m_platforms.at(idx)->m_games.isEmpty() ? -1 : 0;

    emit currentPlatformChanged();
    emit currentGameChanged();
}

void ApiObject::setCurrentGameIndex(int idx)
{
    Q_ASSERT(m_current_platform_idx < m_platforms.count());
    if (m_current_game_idx == idx ||
        m_current_platform_idx < 0 ||
        idx < 0 || idx >= m_platforms.at(m_current_platform_idx)->m_games.count())
        return;

    m_current_game_idx = idx;
    emit currentPlatformChanged();
}
