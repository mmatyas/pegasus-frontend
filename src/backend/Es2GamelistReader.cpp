#include "Es2GamelistReader.h"

#include "Es2Assets.h"
#include "Model.h"
#include "Utils.h"

#include <QDir>
#include <QDebug>


QXmlStreamReader Es2GamelistReader::xml;

QVector<Model::Game*> Es2GamelistReader::read(Model::Platform* platform)
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

QString Es2GamelistReader::findGamelist(const Model::Platform* platform)
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

Model::Game* Es2GamelistReader::readGame(Model::Platform* platform)
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

void Es2GamelistReader::parseGamePath(Model::Game* game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "path");
    game->m_rom_path = xml.readElementText();

    // find out the base name of the rom
    if (!game->m_rom_path.isEmpty()) {
        const QFileInfo rom_path_info(game->m_rom_path);
        game->m_rom_filename = rom_path_info.completeBaseName();
    }
}

void Es2GamelistReader::parseGameName(Model::Game* game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "name");
    game->m_title = xml.readElementText();
}

void Es2GamelistReader::parseGameDescription(Model::Game* game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "desc");
    game->m_description = xml.readElementText();
}

void Es2GamelistReader::parseGameDeveloper(Model::Game* game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "developer");
    game->m_developer = xml.readElementText();
}

void Es2GamelistReader::findGameAssets(Model::Platform* platform, Model::Game* game)
{
    using AssetType = Es2Assets::AssetType;

    Model::GameAssets& assets = *game->m_assets;

    assets.box_front = Es2Assets::find(AssetType::BOX_FRONT, platform, game);
    assets.logo = Es2Assets::find(AssetType::LOGO, platform, game);

    // TODO: support multiple
    assets.screenshot_list << Es2Assets::find(AssetType::SCREENSHOT, platform, game);
    assets.video_list << Es2Assets::find(AssetType::VIDEO, platform, game);
}
