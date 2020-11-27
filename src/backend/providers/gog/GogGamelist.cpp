// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include "Log.h"
#include "Paths.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"
#include "utils/CommandTokenizer.h"
#include "utils/MoveOnly.h"

#include <QDirIterator>
#include <QRegularExpression>
#include <QSettings>
#include <QStringBuilder>


namespace {
// TODO: C++20
struct GogEntry {
    QString id;
    QString name;
    QString exe;
    QString launch_cmd;
    QString workdir;

    GogEntry(QString id, QString name, QString exe, QString launch_cmd, QString workdir)
        : id(std::move(id))
        , name(std::move(name))
        , exe(std::move(exe))
        , launch_cmd(std::move(launch_cmd))
        , workdir(std::move(workdir))
    {}

    MOVE_ONLY(GogEntry)
};

bool invalid_entry(const GogEntry& entry)
{
    return entry.name.isEmpty()
        || entry.launch_cmd.isEmpty()
        || entry.exe.isEmpty()
        || !QFileInfo::exists(entry.exe);
}

std::vector<GogEntry> find_game_entries(const HashMap<QString, std::vector<QString>>& options)
{
    std::vector<GogEntry> entries;

#ifdef Q_OS_WIN
    Q_UNUSED(options);

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

#ifdef Q_OS_LINUX
    const std::vector<QString> directories = [&options]{
        std::vector<QString> out {
            paths::homePath() + QStringLiteral("/GOG Games"),
        };
        const auto directories_it = options.find(QLatin1String("directories"));
        if (directories_it != options.cend())
            out.insert(out.end(), directories_it->second.cbegin(), directories_it->second.cend());

        return out;
    }();

    constexpr auto dir_filters = QDir::Dirs | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::FollowSymlinks;
    const QRegularExpression re_numeric(QStringLiteral("^\\d+$"));

    for (const QString& root : directories) {
        QDirIterator dir_it(root, dir_filters, dir_flags);
        while (dir_it.hasNext()) {
            const QString gamedir(dir_it.next());

            const QString launcher_path(gamedir + QStringLiteral("/start.sh"));
            const QFileInfo launcher_file(launcher_path);
            if (!launcher_file.exists() || !launcher_file.isFile() || !launcher_file.isExecutable())
                continue;

            GogEntry entry {
                QString(),
                dir_it.fileName(),
                launcher_path,
                launcher_path,
                dir_it.filePath(),
            };

            const QString gameinfo_path(gamedir + QStringLiteral("/gameinfo"));
            QFile config_file(gameinfo_path);
            if (config_file.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream stream(&config_file);
                QString line;
                unsigned short lineno = 0;
                while (stream.readLineInto(&line, 256) && lineno <= 3) {
                    if (lineno == 0 && !line.isEmpty())
                        entry.name = line;

                    if (lineno == 3 && re_numeric.match(line).hasMatch())
                        entry.id = line;

                    lineno++;
                }
            }

            entries.emplace_back(std::move(entry));
        }
    }
#endif


    entries.erase(std::remove_if(entries.begin(), entries.end(), invalid_entry), entries.end());
    return entries;
}

HashMap<QString, model::Game*> register_game_entries(
    const std::vector<GogEntry>& gogentries,
    model::Collection& collection,
    providers::SearchContext& sctx)
{
    // NOTE: Games without known GOG ID will be present in SCTX, but not in this output map
    HashMap<QString, model::Game*> gogid_map;

    for (const GogEntry& gogentry : gogentries) {
        QFileInfo finfo(gogentry.exe);

        model::Game* game_ptr = sctx.game_by_filepath(finfo.canonicalFilePath());
        if (!game_ptr) {
            game_ptr = sctx.create_game_for(collection);
            sctx.game_add_filepath(*game_ptr, finfo.canonicalFilePath());
        }

        (*game_ptr)
            .setTitle(gogentry.name)
            .setLaunchCmd(::utils::escape_command(gogentry.launch_cmd))
            .setLaunchWorkdir(gogentry.workdir);

        if (!gogentry.id.isEmpty())
            gogid_map.emplace(gogentry.id, game_ptr);
    }

    return gogid_map;
}
} // namespace


namespace providers {
namespace gog {

Gamelist::Gamelist(QString log_tag)
    : m_log_tag(std::move(log_tag))
{}

HashMap<QString, model::Game*> Gamelist::find(
    const HashMap<QString, std::vector<QString>>& options,
    model::Collection& collection,
    providers::SearchContext& sctx) const
{
    const std::vector<GogEntry> gogentries = find_game_entries(options);
    Log::info(m_log_tag, tr_log("%1 games found").arg(QString::number(gogentries.size())));

    return register_game_entries(gogentries, collection, sctx);
}

} // namespace steam
} // namespace providers
