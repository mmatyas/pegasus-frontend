// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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

#include "GogCommon.h"
#include "LocaleUtils.h"
#include "Paths.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"
#include "utils/CommandTokenizer.h"
#include "utils/MoveOnly.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>
#include <QSettings>
#include <QStringBuilder>


namespace {
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

    constexpr auto dir_filters = QDir::Dirs | QDir::NoDotDot;
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

    return entries;
}

bool invalid_entry(const GogEntry& entry)
{
    return entry.name.isEmpty()
        || entry.launch_cmd.isEmpty()
        || entry.exe.isEmpty()
        || !QFileInfo::exists(entry.exe);
}

void register_entries(const std::vector<GogEntry>& entries,
                      providers::PendingCollection& collection,
                      providers::SearchContext& sctx,
                      HashMap<size_t, QString>& gogids)
{
    for (const GogEntry& entry : entries) {
        QFileInfo finfo(entry.exe);

        auto slot = sctx.add_or_create_game_from_file(std::move(finfo), collection);
        model::Game& game = slot.inner();
        game.setTitle(entry.name);
        game.setLaunchCmd(::utils::escape_command(entry.launch_cmd));
        game.setLaunchWorkdir(entry.workdir);

        if (!entry.id.isEmpty())
            gogids.emplace(slot.id(), entry.id);
    }
}
} // namespace


namespace providers {
namespace gog {

Gamelist::Gamelist(QObject* parent)
    : QObject(parent)
{}

void Gamelist::find(providers::SearchContext& sctx,
                    HashMap<size_t, QString>& gogids,
                    const HashMap<QString, std::vector<QString>>& options)
{
    static constexpr auto MSG_PREFIX = "GOG:";


    std::vector<GogEntry> entries(find_game_entries(options));
    entries.erase(std::remove_if(entries.begin(), entries.end(), invalid_entry), entries.end());

    qInfo().noquote() << MSG_PREFIX << tr_log("%1 games found").arg(entries.size());
    if (entries.empty())
        return;

    PendingCollection& coll = sctx.get_or_create_collection(gog_tag());

    const size_t game_count_before = sctx.games().size();
    register_entries(entries, coll, sctx, gogids);
    if (game_count_before != sctx.games().size())
        emit gameCountChanged(static_cast<int>(sctx.games().size()));
}

} // namespace steam
} // namespace providers
