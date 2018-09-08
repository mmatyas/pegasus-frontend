// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#include "GogGamelist.h"

#include "LocaleUtils.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"
#include "utils/MoveOnly.h"

#include <QSettings>
#include <QStringBuilder>
#include <QDebug>


namespace {
struct GogEntry {
    QString id;
    QString name;
    QString exe;
    QString launch_cmd;
    QString workdir;

    GogEntry(QString id, QString name, QString exe, QString launch_cmd, QString workdir)
        : id(id), name(name), exe(exe), launch_cmd(launch_cmd), workdir(workdir)
    {}

    MOVE_ONLY(GogEntry)
};

std::vector<GogEntry> find_game_entries()
{
    std::vector<GogEntry> entries;

#ifdef Q_OS_WIN
    QSettings reg_base(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\GOG.com\\Games"),
                       QSettings::NativeFormat);

    const QStringList reg_games = reg_base.childGroups();
    for (const QString& reg_game : reg_games) {
        reg_base.beginGroup(reg_game);

        entries.emplace_back(
            reg_base.value(QStringLiteral("gameID")).toString(),
            reg_base.value(QStringLiteral("gameName")).toString(),
            reg_base.value(QStringLiteral("exe")).toString(),
            reg_base.value(QStringLiteral("launchCommand")).toString(),
            reg_base.value(QStringLiteral("workingDir")).toString()
        );

        reg_base.endGroup();
    }
#endif

    return entries;
}

bool invalid_entry(const GogEntry& entry)
{
    return entry.id.isEmpty()
        || entry.name.isEmpty()
        || entry.launch_cmd.isEmpty()
        || entry.exe.isEmpty()
        || !QFileInfo::exists(entry.exe);
}

void register_entries(const std::vector<GogEntry>& entries,
                      HashMap<QString, modeldata::Game>& games,
                      std::vector<QString>& collection_childs)
{
    for (const GogEntry& entry : entries) {
        QFileInfo finfo(entry.exe);
        const QString game_key = finfo.canonicalFilePath();

        if (!games.count(game_key)) {
            modeldata::Game game(std::move(finfo));
            game.title = entry.name;
            game.launch_cmd = '"' % entry.launch_cmd % '"';
            game.launch_workdir = entry.workdir;

            games.emplace(game_key, std::move(game));
        }

        collection_childs.emplace_back(game_key);
    }
}
} // namespace


namespace providers {
namespace gog {

Gamelist::Gamelist(QObject* parent)
    : QObject(parent)
{}

void Gamelist::find(HashMap<QString, modeldata::Game>& games,
                    HashMap<QString, modeldata::Collection>& collections,
                    HashMap<QString, std::vector<QString>>& collection_childs)
{
    static constexpr auto MSG_PREFIX = "GOG:";
    const QString GOG_TAG(QStringLiteral("GOG"));


    std::vector<GogEntry> entries(find_game_entries());
    entries.erase(std::remove_if(entries.begin(), entries.end(), invalid_entry), entries.end());
    if (entries.empty()) {
        qInfo().noquote() << MSG_PREFIX << tr_log("no games found");
        return;
    }


    if (!collections.count(GOG_TAG)) {
        modeldata::Collection collection(GOG_TAG);
        collection.setShortName(GOG_TAG);
        //collection.source_dirs.append(installdirs);
        collections.emplace(GOG_TAG, std::move(collection));
    }

    std::vector<QString>& childs = collection_childs[GOG_TAG];

    const size_t game_count_before = games.size();
    register_entries(entries, games, childs);
    if (game_count_before != games.size())
        emit gameCountChanged(static_cast<int>(games.size()));
}

} // namespace steam
} // namespace providers
