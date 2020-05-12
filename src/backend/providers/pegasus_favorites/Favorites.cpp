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

#include "AppSettings.h"
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
    : Provider(QLatin1String("pegasus_favorites"), QStringLiteral("Favorites"), INTERNAL | PROVIDES_DYNDATA, parent)
{}

Provider& Favorites::load() {
    return load_with_dbpath(default_db_path());
}
Provider& Favorites::load_with_dbpath(QString db_path) {
    m_db_path = std::move(db_path);
    return *this;
}
Provider& Favorites::unload() {
    m_db_path.clear();
    return *this;
}

Provider& Favorites::findDynamicData(const QVector<model::Collection*>&,
                                     const QVector<model::Game*>&,
                                     const HashMap<QString, model::GameFile*>& path_map)
{
    if (!QFileInfo::exists(m_db_path))
        return *this;

    QFile db_file(m_db_path);
    if (!db_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("could not open `%1` for reading, favorites are not loaded.")
                      .arg(m_db_path);
        return *this;
    }

    QTextStream db_stream(&db_file);
    QString line;
    while (db_stream.readLineInto(&line)) {
        if (line.startsWith('#'))
            continue;

        const QString path = QFileInfo(paths::writableConfigDir(), line).canonicalFilePath();
        if (path_map.count(path)) {
            auto parent = static_cast<model::Game* const>(path_map.at(path)->parent());
            parent->setFavorite(true);
        }
    }

    return *this;
}

void Favorites::onGameFavoriteChanged(const QVector<model::Game*>& game_list)
{
    const QMutexLocker lock(&m_task_guard);
    const QDir config_dir(paths::writableConfigDir());

    m_pending_task.clear();
    m_pending_task << QStringLiteral("# List of favorites, one path per line");
    for (const model::Game* const game : game_list) {
        if (game->isFavorite()) {
            for (const model::GameFile* const file : game->filesConst()) {
                const QString full_path = file->fileinfo().canonicalFilePath();
                const QString written_path = AppSettings::general.portable
                    ? config_dir.relativeFilePath(full_path)
                    : full_path;
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
