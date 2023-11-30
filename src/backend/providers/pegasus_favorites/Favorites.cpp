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


#include "Favorites.h"

#include "AppSettings.h"
#include "Log.h"
#include "Paths.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "providers/SearchContext.h"
#include "utils/PathTools.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QtConcurrent/QtConcurrent>


namespace {
QString default_db_path()
{
    return paths::writableConfigDir() + QStringLiteral("/favorites.txt");
}
} // namespace


namespace providers {
namespace favorites {

Favorites::Favorites(QObject* parent)
    : Favorites(default_db_path(), parent)
{}

Favorites::Favorites(QString db_path, QObject* parent)
    : Provider(QLatin1String("pegasus_favorites"), QStringLiteral("Pegasus Favorites"), PROVIDER_FLAG_INTERNAL | PROVIDER_FLAG_HIDE_PROGRESS, parent)
    , m_db_path(std::move(db_path))
{}

Provider& Favorites::run(SearchContext& sctx)
{
    if (!QFileInfo::exists(m_db_path))
        return *this;

    QFile db_file(m_db_path);
    if (!db_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Log::error(display_name(), LOGMSG("Could not open `%1` for reading, favorites not loaded").arg(m_db_path));
        return *this;
    }

    const QDir base_dir = QFileInfo(m_db_path).dir();

    QTextStream db_stream(&db_file);
    db_stream.setCodec("UTF-8");

    QString line;
    while (db_stream.readLineInto(&line)) {
        if (line.startsWith('#'))
            continue;

        model::Game* game_ptr = sctx.game_by_uri(line);
        if (!game_ptr) {
            const QString path = ::clean_abs_path(QFileInfo(base_dir, line));
            game_ptr = sctx.game_by_filepath(path);
        }

        if (game_ptr)
            game_ptr->setFavorite(true);
    }

    return *this;
}

void Favorites::onGameFavoriteChanged(const std::vector<model::Game*>& game_list)
{
    const QMutexLocker lock(&m_task_guard);
    const QDir config_dir(paths::writableConfigDir());

    m_pending_task.clear();
    m_pending_task << QStringLiteral("# List of favorites, one path per line");
    for (const model::Game* const game : game_list) {
        if (game->isFavorite()) {
            for (const model::GameFile* const file : game->filesModel()->entries()) {
                QString written_path;
                if (!file->fileinfo().exists()) {
                    written_path = file->path();
                } else {
                    const QString full_path = ::clean_abs_path(file->fileinfo());
                    written_path = AppSettings::general.portable
                         ? config_dir.relativeFilePath(full_path)
                         : full_path;
                }
                if (Q_LIKELY(!written_path.isEmpty()))
                    m_pending_task << written_path;

            }
        }
    }

    if (m_active_task.isEmpty())
        start_processing();
}

void Favorites::start_processing()
{
    m_active_task = m_pending_task;
    m_pending_task.clear();

    QtConcurrent::run([this]{
        emit startedWriting();

        while (true) {
            QFile db_file(m_db_path);
            if (!db_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                Log::error(display_name(), LOGMSG("Could not open `%1` for writing, favorites are not saved")
                    .arg(m_db_path));
                break;
            }

            QTextStream db_stream(&db_file);
            db_stream.setCodec("UTF-8");

            for (const QString& fav : qAsConst(m_active_task))
                db_stream << fav << Qt::endl;

            QMutexLocker lock(&m_task_guard);
            m_active_task.clear();
            if (m_pending_task.isEmpty())
                break;

            m_active_task = m_pending_task;
            m_pending_task.clear();
        }

        emit finishedWriting();
    });
}

} // namespace favorites
} // namespace providers
