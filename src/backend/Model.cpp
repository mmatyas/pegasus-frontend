#include "Model.h"

#include <QDebug>


namespace Model {

GameAssets::GameAssets(QObject* parent)
    : QObject(parent)
{}

Game::Game(QObject* parent)
    : QObject(parent)
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
