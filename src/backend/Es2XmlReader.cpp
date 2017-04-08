#include "Es2XmlReader.h"

#include "Model.h"
#include "Utils.h"

#include <QDir>
#include <QDebug>


QXmlStreamReader Es2XmlReader::xml;

bool Es2XmlReader::read(Model::PlatformModel& platform_model)
{
    QVector<Model::PlatformItemPtr> platforms = readSystemsFile();
    if (xml.error()) {
        qWarning() << xml.errorString();
        return false;
    }

    for (auto& platform : platforms) {
        QVector<Model::GameItemPtr> games = readGamelistFile(platform);
        if (xml.error()) {
            // qWarning() << xml.errorString();
            continue;
        }

        for (Model::GameItemPtr& game : games) {
            findGameAssets(platform, game);
            platform->game_model.append(game);
        }

        if (platform->game_model.rowCount() > 0)
            platform_model.append(platform);
    }

    return true;
}

QVector<Model::PlatformItemPtr> Es2XmlReader::readSystemsFile()
{
    QString systemscfg_path = findSystemsCfg();
    if (systemscfg_path.isEmpty()) {
        xml.raiseError(QObject::tr("ES2 system config not found"));
        return {};
    }

    QFile systemscfg(systemscfg_path);
    if (!systemscfg.open(QIODevice::ReadOnly)) {
        xml.raiseError(QObject::tr("Could not open `%1`").arg(systemscfg_path));
        return {};
    }

    QVector<Model::PlatformItemPtr> platforms;

    xml.setDevice(&systemscfg);
    if (xml.readNextStartElement()) {
        if (xml.name() != "systemList")
            xml.raiseError(QObject::tr("`%1` does not start with a `<systemList>` node!"));
        else {
            while (xml.readNextStartElement()) {
                if (xml.name() == "system") {
                    Model::PlatformItemPtr platform = readSystem();
                    if (!platform->short_name.isEmpty())
                        platforms.push_back(platform);
                }
                else
                    xml.skipCurrentElement();
            }
        }
    }

    return platforms;
}

QString Es2XmlReader::findSystemsCfg()
{
    static const QString FOUND_MSG = "Found `%1`";
    static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    static const QVector<QString> possible_paths = {
        QDir::homePath() + "/.config/emulationstation/es_systems.cfg",
        QDir::homePath() + "/.emulationstation/es_systems.cfg",
        "/etc/emulationstation/es_systems.cfg",
    };

    for (const auto& path : possible_paths) {
        if (validFile(path)) {
            qInfo() << FOUND_MSG.arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

Model::PlatformItemPtr Es2XmlReader::readSystem()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    Model::PlatformItemPtr platform(new Model::PlatformItem());

    while (xml.readNextStartElement()) {
        if (xml.name() == "name")
            parseSystemShortName(platform);
        else if (xml.name() == "path")
            parseSystemRomDirPath(platform);
        else if (xml.name() == "command")
            parseSystemRunCmd(platform);
        else
            xml.skipCurrentElement();
    }

    return platform;
}

void Es2XmlReader::parseSystemShortName(Model::PlatformItemPtr& platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "name");
    platform->short_name = xml.readElementText();
}

void Es2XmlReader::parseSystemRomDirPath(Model::PlatformItemPtr& platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "path");
    platform->rom_dir_path = xml.readElementText()
        .replace("\\", "/")
        .replace("~", QDir::homePath());
}

void Es2XmlReader::parseSystemRunCmd(Model::PlatformItemPtr& platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "command");
    platform->launch_cmd = xml.readElementText();
}

QVector<Model::GameItemPtr> Es2XmlReader::readGamelistFile(const Model::PlatformItemPtr& platform)
{
    QString gamelist_path = findGamelist(platform);
    if (gamelist_path.isEmpty()) {
        xml.raiseError(QObject::tr("Gamelist for platform `%1` not found").arg(platform->short_name));
        return {};
    }

    QFile gamelist(gamelist_path);
    if (!gamelist.open(QIODevice::ReadOnly)) {
        xml.raiseError(QObject::tr("Could not open `%1`").arg(gamelist_path));
        return {};
    }

    QVector<Model::GameItemPtr> games;

    xml.setDevice(&gamelist);
    if (xml.readNextStartElement()) {
        if (xml.name() != "gameList")
            xml.raiseError(QObject::tr("`%1` does not start with a `<gameList>` node!"));
        else {
            while (xml.readNextStartElement()) {
                if (xml.name() == "game") {
                    Model::GameItemPtr game = readGame();
                    if (!game->rom_path.isEmpty())
                        games.push_back(game);
                }
                else
                    xml.skipCurrentElement();
            }
        }
    }

    return games;
}

QString Es2XmlReader::findGamelist(const Model::PlatformItemPtr& platform)
{
    static constexpr auto FILENAME = "/gamelist.xml";
    static const QString FOUND_MSG = "Found `%1`";
    static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    if (platform->short_name.isEmpty())
        return "";

    const QString path_suffix = "/gamelists/" + platform->short_name + FILENAME;
    const QVector<QString> possible_paths = {
        platform->rom_dir_path + FILENAME,
        QDir::homePath() + "/.config/emulationstation" + path_suffix,
        QDir::homePath() + "/.emulationstation" + path_suffix,
        "/etc/emulationstation" + path_suffix,
    };

    for (const auto& path : possible_paths) {
        if (validFile(path)) {
            qInfo() << FOUND_MSG.arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

Model::GameItemPtr Es2XmlReader::readGame()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    Model::GameItemPtr game(new Model::GameItem());

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

void Es2XmlReader::parseGamePath(Model::GameItemPtr& game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "path");
    game->rom_path = xml.readElementText();

    // find out the base name of the rom
    if (!game->rom_path.isEmpty()) {
        const QFileInfo rom_path_info(game->rom_path);
        game->rom_filename = rom_path_info.completeBaseName();
    }
}

void Es2XmlReader::parseGameName(Model::GameItemPtr& game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "name");
    game->title = xml.readElementText();
}

void Es2XmlReader::parseGameDescription(Model::GameItemPtr& game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "desc");
    game->description = xml.readElementText();
}

void Es2XmlReader::parseGameDeveloper(Model::GameItemPtr& game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "developer");
    game->developer = xml.readElementText();
}

void Es2XmlReader::findGameAssets(Model::PlatformItemPtr& platform, Model::GameItemPtr& game)
{
    using AssetType = Es2Assets::AssetType;

    Model::GameAssets& assets = game->assets;

    assets.box_front = Es2Assets::find(AssetType::BOX_FRONT, platform, game);
    assets.logo = Es2Assets::find(AssetType::LOGO, platform, game);

    // TODO: support multiple
    assets.screenshot_list << Es2Assets::find(AssetType::SCREENSHOT, platform, game);
    assets.video_list << Es2Assets::find(AssetType::VIDEO, platform, game);
}


QVector<QString> Es2Assets::possibleSuffixes(AssetType asset_type)
{
    static const QMap<AssetType, QVector<QString>> suffix_map = {
        { AssetType::BOX_FRONT, { "-boxFront", "-box_front", "-boxart2D", "" } },
        { AssetType::LOGO, { "-logo" } },
        { AssetType::SCREENSHOT, { "-screenshot" } },
        { AssetType::VIDEO, { "-video" } },
    };

    Q_ASSERT(suffix_map.contains(asset_type));
    return suffix_map.value(asset_type);
}

QVector<QString> Es2Assets::possibleExtensions(AssetType asset_type)
{
    static const QVector<QString> image_exts = { ".png", ".jpg" };
    static const QVector<QString> video_exts = { ".webm", ".mp4", ".avi" };

    switch (asset_type) {
        case AssetType::BOX_FRONT:
        case AssetType::LOGO:
        case AssetType::SCREENSHOT:
            return image_exts;
        case AssetType::VIDEO:
            return video_exts;
    }

    Q_ASSERT(false);
    return {};
}

QString Es2Assets::find(AssetType asset_type,
                        const Model::PlatformItemPtr& platform,
                        const Model::GameItemPtr& game)
{
    if (platform->short_name.isEmpty() ||
        platform->rom_dir_path.isEmpty() ||
        game->rom_filename.isEmpty())
        return QString();

    // check all possible [basedir] + [subdir] + [suffix] + [extension]
    // combination when searching for an asset

    const QVector<QString> possible_suffixes = possibleSuffixes(asset_type);
    const QVector<QString> possible_exts = possibleExtensions(asset_type);
    Q_ASSERT(!possible_suffixes.isEmpty());
    Q_ASSERT(!possible_exts.isEmpty());

    // in portable mode, the files are next to the roms under ./media/,
    // but for regular installations, it's under ./downloaded_images/
    const QString subdir = "/downloaded_images/" + platform->short_name + "/" + game->rom_filename;
    const QVector<QString> possible_base_paths = {
        platform->rom_dir_path + "/media/" + game->rom_filename, // portable edition
        platform->rom_dir_path + subdir,
        QDir::homePath() + "/.config/emulationstation" + subdir,
        QDir::homePath() + "/.emulationstation" + subdir,
        "/etc/emulationstation" + subdir,
    };

    // check every combination until there's a match
    for (const auto& base_path : possible_base_paths) {
        for (const auto& asset_suffix : possible_suffixes) {
            for (const auto& ext : possible_exts) {
                if (validFile(base_path + asset_suffix + ext))
                    return base_path + asset_suffix + ext;
            }
        }
    }

    return QString();
}
