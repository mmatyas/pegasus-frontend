#include "Es2Gamelist.h"

#include "Model.h"
#include "Utils.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QStringBuilder>


namespace Es2 {

QXmlStreamReader Gamelist::xml;

void Gamelist::read(const Model::Platform& platform)
{
    // reset
    xml.clear();

    // find the file
    const QString xml_path = findGamelistFile(platform);
    if (xml_path.isEmpty()) {
        qWarning().noquote() << QObject::tr("ES2 gamelist for platform `%1` not found")
                                .arg(platform.m_short_name);
        return;
    }

    // open the file
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << QObject::tr("Could not open `%1`").arg(xml_path);
        return;
    }

    // parse the file
    parseGamelistFile(xml_file, platform);
    if (xml.error())
        qWarning().noquote() << xml.errorString();
}

QString Gamelist::findGamelistFile(const Model::Platform& platform)
{
    Q_ASSERT(!platform.m_short_name.isEmpty());

    static constexpr auto FILENAME = "/gamelist.xml";
    static const auto FOUND_MSG = QObject::tr("Found `%1`");
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";


    // check portable paths

    const QString portable_path = platform.m_rom_dir_path % FILENAME;
    if (validFile(portable_path)) {
        qInfo().noquote() << FOUND_MSG.arg(portable_path);
        return portable_path;
    }
    // qDebug() << FALLBACK_MSG.arg(path);


    // check ES2-specific paths

    static const QVector<QString> es2_root_paths = {
        QDir::homePath() % "/.config/emulationstation/gamelists/",
        QDir::homePath() % "/.emulationstation/gamelists/",
        "/etc/emulationstation/gamelists/",
    };

    const QString es2_subpath = platform.m_short_name % FILENAME;

    for (const auto& root_path : es2_root_paths) {
        const QString path = root_path % es2_subpath;
        if (validFile(path)) {
            qInfo().noquote() << FOUND_MSG.arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

void Gamelist::parseGamelistFile(QFile& gamelist, const Model::Platform& platform)
{
    Q_ASSERT(gamelist.isOpen() && gamelist.isReadable());
    Q_ASSERT(platform.m_games.count() > 0);

    xml.setDevice(&gamelist);

    // Build a path -> game map for quick access.
    // To find matches between the real files and the ones in the gamelist,
    // their canonical path will be compared.
    QHash<QString, Model::Game*> game_by_path;
    for (Model::Game* game : platform.m_games)
        game_by_path.insert(game->m_rom_path, game);


    // find the root <gameList> element
    if (!xml.readNextStartElement()) {
        xml.raiseError(QObject::tr("Could not parse `%1`").arg(gamelist.fileName()));
        return;
    }
    if (xml.name() != "gameList") {
        xml.raiseError(QObject::tr("`%1` does not have a `<gameList>` root node!")
                       .arg(gamelist.fileName()));
        return;
    }

    // read all <game> nodes
    while (xml.readNextStartElement()) {
        if (xml.name() != "game") {
            xml.skipCurrentElement();
            continue;
        }

        parseGameTag(platform, game_by_path);
    }
}

void Gamelist::parseGameTag(const Model::Platform& platform,
                            QHash<QString, Model::Game*>& game_by_path)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    static const QString PATH_TAG = "path";

    // read all XML fields into a key-value map
    QHash<QString, QString> xml_props;
    while (xml.readNextStartElement())
        xml_props.insert(xml.name().toString(), xml.readElementText());
    if (!xml_props.contains(PATH_TAG))
        return;

    // find the matching game
    // NOTE: every game should appear only once, so we can take() it out of the map
    const QString path = platform.m_rom_dir_path + "/" + xml_props[PATH_TAG];
    const QString canonical_path = QFileInfo(path).canonicalFilePath();
    Model::Game* game = game_by_path.take(canonical_path);
    if (!game)
        return;

    // apply the previously read values

    game->m_description = xml_props.value("desc");
    game->m_developer = xml_props.value("developer");
    game->m_genre = xml_props.value("genre");
    game->m_publisher = xml_props.value("publisher");
    game->m_title = xml_props.value("name");

    parseStoreInt(xml_props.value("players"), game->m_players);
    parseStoreInt(xml_props.value("playcount"), game->m_playcount);

    game->m_lastplayed = QDateTime::fromString(xml_props.value("lastplayed"), Qt::ISODate);

    const QDateTime release_date(QDateTime::fromString(xml_props.value("releasedate"), Qt::ISODate));
    if (release_date.isValid()) {
        const QDate date(release_date.date());
        game->m_year = date.year();
        game->m_month = date.month();
        game->m_day = date.day();
    }
}

} // namespace Es2
