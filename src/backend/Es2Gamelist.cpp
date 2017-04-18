#include "Es2Gamelist.h"

#include "Es2Assets.h"
#include "Model.h"
#include "Utils.h"

#include <QDir>
#include <QDebug>


namespace Es2 {

QString Gamelist::xml_path;
QXmlStreamReader Gamelist::xml;

void Gamelist::read(const Model::Platform& platform)
{
    readGamelistFile(platform);
    if (xml.error())
        qWarning().noquote() << xml.errorString();
}

void Gamelist::readGamelistFile(const Model::Platform& platform)
{
    xml.clear();
    xml_path.clear();

    xml_path = findGamelistFile(platform);
    if (xml_path.isEmpty()) {
        xml.raiseError(QObject::tr("ES2 gamelist for platform `%1` not found")
                       .arg(platform.m_short_name));
        return;
    }

    QFile xml_file(xml_path);
    openGamelistFile(xml_file);
    if (!xml.device())
        return;

    parseGamelistFile(platform);
}

QString Gamelist::findGamelistFile(const Model::Platform& platform)
{
    static constexpr auto FILENAME = "/gamelist.xml";
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    Q_ASSERT(xml_path.isEmpty());
    Q_ASSERT(!platform.m_short_name.isEmpty());

    // the suffix appended to installed locations
    const QString main_suffix = QStringLiteral("/gamelists/%1/%2")
        .arg(platform.m_short_name)
        .arg(FILENAME);

    const QVector<QString> possible_paths = {
        platform.m_rom_dir_path + FILENAME,
        QDir::homePath() + "/.config/emulationstation" + main_suffix,
        QDir::homePath() + "/.emulationstation" + main_suffix,
        "/etc/emulationstation" + main_suffix,
    };

    for (const auto& path : possible_paths) {
        if (validFile(path)) {
            qInfo().noquote() << QStringLiteral("Found `%1`").arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

void Gamelist::openGamelistFile(QFile& xml_file)
{
    Q_ASSERT(!xml.device());
    Q_ASSERT(!xml_path.isEmpty());

    if (!xml_file.open(QIODevice::ReadOnly)) {
        xml.raiseError(QObject::tr("Could not open `%1`").arg(xml_path));
        return;
    }

    xml.setDevice(&xml_file);
}

void Gamelist::parseGamelistFile(const Model::Platform& platform)
{
    Q_ASSERT(xml.device() && !xml.atEnd());

    // Build a path -> game map for quick access.
    // To find matches between the real files and the ones in the gamelist,
    // their canonical path will be compared.
    QHash<QString, Model::Game*> game_by_path;
    for (Model::Game* game : platform.m_games)
        game_by_path.insert(QFileInfo(game->m_rom_path).canonicalFilePath(), game);

    // read the root <gameList> element
    if (xml.readNextStartElement()) {
        if (xml.name() != "gameList") {
            xml.raiseError(QObject::tr("`%1` does not have a `<gameList>` root node!")
                           .arg(xml_path));
        }
        else {
            // read all <game> nodes
            while (xml.readNextStartElement()) {
                if (xml.name() != "game") {
                    xml.skipCurrentElement();
                    continue;
                }

                handleGameTag(platform, game_by_path);
            }
        }
    }
    else {
        xml.raiseError(QObject::tr("Could not parse `%1`").arg(xml_path));
    }
}

void Gamelist::handleGameTag(const Model::Platform& platform,
                             QHash<QString, Model::Game*>& game_by_path)
{
    static constexpr auto PATH_TAG = "path";

    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    QHash<QString, QString> xml_props = readGameProperties();
    if (!xml_props.contains(PATH_TAG))
        return;

    const QString path = platform.m_rom_dir_path + "/" + xml_props[PATH_TAG];
    const QString canonical_path = QFileInfo(path).canonicalFilePath();

    // every game should appear only once, so we can take it out of the map
    Model::Game* target = game_by_path.take(canonical_path);
    if (!target)
        return;

    target->m_description = xml_props.value("desc");
    target->m_developer = xml_props.value("developer");
    target->m_genre = xml_props.value("genre");
    target->m_publisher = xml_props.value("publisher");
    target->m_title = xml_props.value("name");
}

QHash<QString, QString> Gamelist::readGameProperties()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    QHash<QString, QString> properties;

    while (xml.readNextStartElement())
        properties.insert(xml.name().toString(), xml.readElementText());

    return properties;
}

} // namespace Es2
