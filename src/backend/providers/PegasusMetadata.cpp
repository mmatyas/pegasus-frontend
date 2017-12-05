// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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


#include "PegasusMetadata.h"

#include "types/Game.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QStringBuilder>
#include <QUrl>


namespace {
static const QString MEDIA_SUBDIR("/media/");

static const QHash<QString, AssetType> TYPE_BY_SUFFIX = {
    { QStringLiteral("-boxFront"), AssetType::BOX_FRONT },
    { QStringLiteral("-box_front"), AssetType::BOX_FRONT },
    { QStringLiteral("-boxart2D"), AssetType::BOX_FRONT },
    { QStringLiteral("-boxBack"), AssetType::BOX_BACK },
    { QStringLiteral("-box_back"), AssetType::BOX_BACK },
    { QStringLiteral("-boxSpine"), AssetType::BOX_SPINE },
    { QStringLiteral("-box_spine"), AssetType::BOX_SPINE },
    { QStringLiteral("-boxSide"), AssetType::BOX_SPINE },
    { QStringLiteral("-box_side"), AssetType::BOX_SPINE },
    { QStringLiteral("-boxFull"), AssetType::BOX_FULL },
    { QStringLiteral("-box_full"), AssetType::BOX_FULL },
    { QStringLiteral("-box"), AssetType::BOX_FULL },
    { QStringLiteral("-cartridge"), AssetType::CARTRIDGE },
    { QStringLiteral("-disc"), AssetType::CARTRIDGE },
    { QStringLiteral("-cart"), AssetType::CARTRIDGE },
    { QStringLiteral("-logo"), AssetType::LOGO },
    { QStringLiteral("-wheel"), AssetType::LOGO },
    { QStringLiteral("-marquee"), AssetType::MARQUEE },
    { QStringLiteral("-bezel"), AssetType::BEZEL },
    { QStringLiteral("-screenmarquee"), AssetType::BEZEL },
    { QStringLiteral("-border"), AssetType::BEZEL },
    { QStringLiteral("-steam"), AssetType::STEAMGRID },
    { QStringLiteral("-steamgrid"), AssetType::STEAMGRID },
    { QStringLiteral("-grid"), AssetType::STEAMGRID },
    { QStringLiteral("-flyer"), AssetType::FLYER },
    { QStringLiteral("-background"), AssetType::BACKGROUND },
    { QStringLiteral("-music"), AssetType::MUSIC },
    { QStringLiteral("-screenshot"), AssetType::SCREENSHOTS },
    { QStringLiteral("-video"), AssetType::VIDEOS },
};
static const QHash<QString, AssetType> TYPE_BY_EXT = {
    { QStringLiteral("png"), AssetType::BOX_FRONT },
    { QStringLiteral("jpg"), AssetType::BOX_FRONT },
    { QStringLiteral("webm"), AssetType::VIDEOS },
    { QStringLiteral("mp4"), AssetType::VIDEOS },
    { QStringLiteral("avi"), AssetType::VIDEOS },
    { QStringLiteral("mp3"), AssetType::MUSIC },
    { QStringLiteral("ogg"), AssetType::MUSIC },
    { QStringLiteral("wav"), AssetType::MUSIC },
};

const QStringList& allowedAssetExts(AssetType type)
{
    static const QStringList empty_list({});
    static const QStringList image_exts = { "png", "jpg" };
    static const QStringList video_exts = { "webm", "mp4", "avi" };
    static const QStringList audio_exts = { "mp3", "ogg", "wav" };

    switch (type) {
        case AssetType::UNKNOWN:
            return empty_list;
        case AssetType::VIDEOS:
            return video_exts;
        case AssetType::MUSIC:
            return audio_exts;
        default:
            return image_exts;
    }
}
AssetType findAssetType(const QString& suffix, const QString& ext)
{
    if (suffix.isEmpty()) {
        return TYPE_BY_EXT.contains(ext)
            ? TYPE_BY_EXT[ext]
            : AssetType::UNKNOWN;
    }

    if (TYPE_BY_SUFFIX.contains(suffix)
        && allowedAssetExts(TYPE_BY_SUFFIX[suffix]).contains(ext)) {
        return TYPE_BY_SUFFIX[suffix];
    }

    return AssetType::UNKNOWN;
}

} // namespace

namespace providers {

void PegasusMetadata::fill(const QHash<QString, Types::Game*>& games,
                           const QHash<QString, Types::Collection*>&,
                           const QVector<QString>& metadata_dirs)
{
    QHash<QString, Types::Game*> games_shortpath;
    games_shortpath.reserve(games.size());
    for (Types::Game* const game : games) {
        const QString shortpath = game->m_fileinfo.canonicalPath() % '/' % game->m_fileinfo.completeBaseName();
        games_shortpath.insert(shortpath, game);
    }

    static constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    static constexpr auto dir_flags = QDirIterator::FollowSymlinks;

    for (const QString& dir_base : metadata_dirs) {
        const QString media_dir = dir_base + MEDIA_SUBDIR;

        QDirIterator dir_it(media_dir, dir_filters, dir_flags);
        while (dir_it.hasNext()) {
            dir_it.next();
            QFileInfo fileinfo = dir_it.fileInfo();

            int last_dash = fileinfo.completeBaseName().lastIndexOf(QChar('-'));
            if (last_dash == -1)
                last_dash = fileinfo.completeBaseName().size();

            // NOTE
            // - left(strlen) returns the whole string
            // - mid(strlen) returns a null string

            QString shortpath = dir_base % '/' % fileinfo.completeBaseName().leftRef(last_dash);
            if (!games_shortpath.contains(shortpath)) {
                // this also happens when the filename part contains a dash, but has no suffix
                shortpath += fileinfo.completeBaseName().midRef(last_dash);
                if (!games_shortpath.contains(shortpath))
                    continue;
                last_dash = fileinfo.completeBaseName().size();
            }

            const QString suffix = fileinfo.completeBaseName().mid(last_dash);
            const AssetType type = findAssetType(suffix, fileinfo.suffix());
            if (type == AssetType::UNKNOWN)
                continue;

            const bool is_multi = Assets::multiTypes.contains(type);
            QString url = QUrl::fromLocalFile(dir_it.filePath()).toString();

            Types::Game* const game = games_shortpath[shortpath];
            if (is_multi && !game->assets().m_multi_assets[type].contains(url))
                game->assets().appendMulti(type, std::move(url));
            else if (!is_multi && game->assets().m_single_assets[type].isEmpty())
                game->assets().setSingle(type, std::move(url));
        }
    }
}

} // namespace providers
