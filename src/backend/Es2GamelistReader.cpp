#include "Es2GamelistReader.h"

#include "Es2Assets.h"
#include "Model.h"
#include "Utils.h"

#include <QDir>
#include <QDebug>


namespace Es2 {

QXmlStreamReader Gamelist::xml;

QVector<Model::Game*> Gamelist::read(Model::Platform* platform)
{
    QString gamelist_path = findGamelist(platform);
    if (gamelist_path.isEmpty()) {
        // qWarning().noquote() << QObject::tr("Gamelist for platform `%1` not found").arg(platform->short_name);
        return {};
    }

    QFile gamelist(gamelist_path);
    if (!gamelist.open(QIODevice::ReadOnly)) {
        xml.raiseError(QObject::tr("Could not open `%1`").arg(gamelist_path));
        return {};
    }

    QVector<Model::Game*> games;

    xml.setDevice(&gamelist);
    if (xml.readNextStartElement()) {
        if (xml.name() != "gameList")
            xml.raiseError(QObject::tr("`%1` does not start with a `<gameList>` node!"));
        else {
            while (xml.readNextStartElement()) {
                if (xml.name() == "game") {
                    Model::Game* game = readGame(platform);
                    if (!game->m_rom_path.isEmpty())
                        games.push_back(game);
                }
                else
                    xml.skipCurrentElement();
            }
        }
    }

    return games;
}

QString Gamelist::findGamelist(const Model::Platform* platform)
{
    static constexpr auto FILENAME = "/gamelist.xml";
    static const QString FOUND_MSG = "Found `%1`";
    static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    if (platform->m_short_name.isEmpty())
        return "";

    const QString path_suffix = "/gamelists/" + platform->m_short_name + FILENAME;
    const QVector<QString> possible_paths = {
        platform->m_rom_dir_path + FILENAME,
        QDir::homePath() + "/.config/emulationstation" + path_suffix,
        QDir::homePath() + "/.emulationstation" + path_suffix,
        "/etc/emulationstation" + path_suffix,
    };

    for (const auto& path : possible_paths) {
        if (validFile(path)) {
            qInfo().noquote() << FOUND_MSG.arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

Model::Game* Gamelist::readGame(Model::Platform* platform)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    Model::Game* game = new Model::Game(platform);

    while (xml.readNextStartElement()) {
        if (xml.name() == "path")
            parseGamePath(game);
        else if (xml.name() == "name")
            parseGameName(game);
        else if (xml.name() == "desc")
            parseGameDescription(game);
        else if (xml.name() == "developer")
            parseGameDeveloper(game);
        else
            xml.skipCurrentElement();
    }

    return game;
}

void Gamelist::parseGamePath(Model::Game* game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "path");
    game->m_rom_path = xml.readElementText();

    // find out the base name of the rom
    if (!game->m_rom_path.isEmpty()) {
        const QFileInfo rom_path_info(game->m_rom_path);
        game->m_rom_filename = rom_path_info.completeBaseName();
    }
}

void Gamelist::parseGameName(Model::Game* game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "name");
    game->m_title = xml.readElementText();
}

void Gamelist::parseGameDescription(Model::Game* game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "desc");
    game->m_description = xml.readElementText();
}

void Gamelist::parseGameDeveloper(Model::Game* game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "developer");
    game->m_developer = xml.readElementText();
}

void Gamelist::findGameAssets(Model::Platform* platform, Model::Game* game)
{
    using Asset = Es2::Assets::AssetType;

    Model::GameAssets& assets = *game->m_assets;

    assets.m_box_front = Es2::Assets::find(Asset::BOX_FRONT, platform, game);
    assets.m_box_back = Es2::Assets::find(Asset::BOX_BACK, platform, game);
    assets.m_box_spine = Es2::Assets::find(Asset::BOX_SPINE, platform, game);
    assets.m_box_full = Es2::Assets::find(Asset::BOX_FULL, platform, game);
    assets.m_cartridge = Es2::Assets::find(Asset::CARTRIDGE, platform, game);
    assets.m_logo = Es2::Assets::find(Asset::LOGO, platform, game);
    assets.m_marquee = Es2::Assets::find(Asset::MARQUEE, platform, game);
    assets.m_bezel = Es2::Assets::find(Asset::BEZEL, platform, game);
    assets.m_gridicon = Es2::Assets::find(Asset::STEAMGRID, platform, game);
    assets.m_flyer = Es2::Assets::find(Asset::FLYER, platform, game);

    // TODO: support multiple
    assets.m_fanarts << Es2::Assets::find(Asset::FANARTS, platform, game);
    assets.m_screenshots << Es2::Assets::find(Asset::SCREENSHOTS, platform, game);
    assets.m_videos << Es2::Assets::find(Asset::VIDEOS, platform, game);
}

} // namespace Es2
