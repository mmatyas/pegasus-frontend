#include "Collection.h"

#include "Game.h"


namespace modeldata {

Collection::Collection(QString name)
    : m_name(std::move(name))
{}

void Collection::setShortName(const QString& str)
{
    Q_ASSERT(!str.isEmpty());
    m_short_name = str.toLower();
}

void Collection::setLaunchCmd(QString str)
{
    Q_ASSERT(!str.isEmpty());
    m_launch_cmd = std::move(str);
}

void Collection::sortGames()
{
    // remove duplicates
    std::sort(m_games.begin(), m_games.end());
    m_games.erase(std::unique(m_games.begin(), m_games.end()), m_games.end());

    // sort by name
    std::sort(m_games.begin(), m_games.end(),
        [](const QSharedPointer<Game>& a, QSharedPointer<Game>& b) {
            return QString::localeAwareCompare(a->fileinfo().completeBaseName(),
                                               b->fileinfo().completeBaseName()) < 0;
        }
    );
}

} // namespace modeldata
