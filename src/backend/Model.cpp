#include "Model.h"

#include <QDebug>
#include <QFileInfo>


namespace Model {

GameAssets::GameAssets(QObject* parent)
    : QObject(parent)
{}

Game::Game(const QString& path, QObject* parent)
    : QObject(parent)
    , m_rom_path(path)
    , m_rom_basename(QFileInfo(m_rom_path).completeBaseName())
    , m_title(m_rom_basename)
    , m_year(0)
    , m_month(0)
    , m_day(0)
    , m_players(1)
    , m_assets(new GameAssets(this))
{}

Platform::Platform(QObject* parent)
    : QObject(parent)
{}

QQmlListProperty<Model::Game> Platform::getGamesProp()
{
    return QQmlListProperty<Model::Game>(this, m_games);
}

} // namespace Model
