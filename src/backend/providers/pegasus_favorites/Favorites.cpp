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


#include "Favorites.h"

#include "LocaleUtils.h"
#include "Paths.h"
#include "model/gaming/Game.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QtConcurrent/QtConcurrent>


namespace {
static constexpr auto MSG_PREFIX = "Favorites:";

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
    : Provider(QStringLiteral("Favorites"), PROVIDES_DYNDATA, parent)
    , m_db_path(std::move(db_path))
{}

void Favorites::findDynamicData(const QVector<model::Collection*>&,
                                const QVector<model::Game*>&,
                                const HashMap<QString, model::GameFile*>& path_map)
{
    if (!QFileInfo::exists(m_db_path))
        return;

    QFile db_file(m_db_path);
    if (!db_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("could not open `%1` for reading, favorites are not loaded.")
                      .arg(m_db_path);
        return;
    }

    QTextStream db_stream(&db_file);
    QString line;
    while (db_stream.readLineInto(&line)) {
        if (line.startsWith('#'))
            continue;

        if (path_map.count(line)) {
            auto parent = static_cast<model::Game* const>(path_map.at(line)->parent());
            parent->setFavorite(true);
        }
    }
}

void Favorites::onGameFavoriteChanged(const QVector<model::Game*>& game_list)
{
    QMutexLocker lock(&m_task_guard);

    m_pending_task.clear();
    m_pending_task << QStringLiteral("# List of favorites, one path per line");
    for (const model::Game* const game : game_list) {
        if (game->favorite()) {
            for (const model::GameFile* const file : game->filesConst()) {
                const QString path = file->data().fileinfo.canonicalFilePath();
                if (Q_LIKELY(!path.isEmpty()))
                    m_pending_task << path;
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
                qWarning().noquote() << MSG_PREFIX
                    << tr_log("could not open `%1` for writing, favorites are not saved.")
                              .arg(m_db_path);
                break;
            }

            QTextStream db_stream(&db_file);
            for (const QString& fav : qAsConst(m_active_task))
                db_stream << fav << endl;

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
