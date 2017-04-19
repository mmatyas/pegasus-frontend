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
    , m_playcount(0)
    , m_assets(new GameAssets(this))
{}

Platform::Platform(const QString& name, const QString& rom_dir_path,
                   const QStringList& rom_filters, const QString& launch_cmd,
                   QObject* parent)
    : QObject(parent)
    , m_short_name(name)
    , m_rom_dir_path(rom_dir_path)
    , m_rom_filters(rom_filters)
    , m_launch_cmd(launch_cmd)
{}

QQmlListProperty<Model::Game> Platform::getGamesProp()
{
    return QQmlListProperty<Model::Game>(this, m_games);
}

} // namespace Model
