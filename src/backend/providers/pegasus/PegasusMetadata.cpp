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


#include "PegasusMetadata.h"

#include "ConfigFile.h"
#include "LocaleUtils.h"
#include "Paths.h"
#include "PegasusAssets.h"
#include "PegasusCommon.h"
#include "Utils.h"
#include "modeldata/gaming/Game.h"

#include <QDebug>
#include <QFileInfo>
#include <QStringBuilder>
#include <QUrl>


namespace {

void add_asset(modeldata::GameAssets& game_assets, const AssetType asset_type, const QString& value,
               const QString& relative_dir)
{
    Q_ASSERT(asset_type != AssetType::UNKNOWN);

    QFileInfo finfo(value);
    if (finfo.isRelative())
        finfo.setFile(relative_dir + '/' + value);

    // FIXME: reduce duplication with pegasus_assets::addAssetToGame

    QString url = QUrl::fromLocalFile(finfo.absoluteFilePath()).toString();
    const bool is_single = asset_is_single(asset_type);
    if (is_single) {
        game_assets.setSingle(asset_type, std::move(url));
    }
    else if (!game_assets.multi(asset_type).contains(url)) {
        game_assets.appendMulti(asset_type, std::move(url));
    }
}

} // namespace


namespace providers {
namespace pegasus {

enum class MetaAttribType : unsigned char {
    TITLE,
    DEVELOPER,
    PUBLISHER,
    GENRE,
    PLAYER_COUNT,
    SHORT_DESC,
    LONG_DESC,
    RELEASE,
    RATING,
    LAUNCH_CMD,
};

PegasusMetadata::PegasusMetadata()
    : m_key_types {
        { QStringLiteral("title"), MetaAttribType::TITLE },
        { QStringLiteral("name"), MetaAttribType::TITLE },
        { QStringLiteral("developer"), MetaAttribType::DEVELOPER },
        { QStringLiteral("developers"), MetaAttribType::DEVELOPER },
        { QStringLiteral("publisher"), MetaAttribType::PUBLISHER },
        { QStringLiteral("publishers"), MetaAttribType::PUBLISHER },
        { QStringLiteral("genre"), MetaAttribType::GENRE },
        { QStringLiteral("genres"), MetaAttribType::GENRE },
        { QStringLiteral("players"), MetaAttribType::PLAYER_COUNT },
        { QStringLiteral("summary"), MetaAttribType::SHORT_DESC },
        { QStringLiteral("description"), MetaAttribType::LONG_DESC },
        { QStringLiteral("release"), MetaAttribType::RELEASE },
        { QStringLiteral("rating"), MetaAttribType::RATING },
        { QStringLiteral("launch"), MetaAttribType::LAUNCH_CMD },
        { QStringLiteral("command"), MetaAttribType::LAUNCH_CMD },
    }
    , m_player_regex(QStringLiteral("^(\\d+)(-(\\d+))?$"))
    , m_rating_percent_regex(QStringLiteral("^\\d+%$"))
    , m_rating_float_regex(QStringLiteral("^\\d(\\.\\d+)?$"))
    , m_release_regex(QStringLiteral("^(\\d{4})(-(\\d{1,2}))?(-(\\d{1,2}))?$"))
{
}

void PegasusMetadata::enhance_in_dirs(const QStringList& dir_list,
                                      HashMap<QString, modeldata::Game>& games,
                                      const HashMap<QString, modeldata::Collection>&,
                                      const HashMap<QString, std::vector<QString>>&) const
{
    pegasus_assets::findAssets(dir_list, games);

    for (const QString& dir_path : dir_list)
        read_metadata_file(dir_path, games);
}


void PegasusMetadata::read_metadata_file(const QString& dir_path,
                                         HashMap<QString, modeldata::Game>& games) const
{
    QString curr_config_path;
    modeldata::Game* curr_game = nullptr;

    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << tr_log("`%1`, line %2: %3").arg(curr_config_path, QString::number(lineno), msg);
    };
    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        if (key == QLatin1String("file")) {
            const QString curr_game_path = dir_path % '/' % val;
            const QFileInfo fileinfo(curr_game_path);
            curr_game = nullptr;

            if (!games.count(fileinfo.canonicalFilePath())) {
                on_error(lineno,
                    tr_log("the game `%1` is either missing or excluded, values for it will be ignored").arg(val));
                return;
            }

            curr_game = &games.at(fileinfo.canonicalFilePath());
            return;
        }
        if (!curr_game) {
            on_error(lineno, tr_log("no file defined yet, entry ignored"));
            return;
        }

        if (key.startsWith(QLatin1String("x-"))) {
            // TODO: unimplemented
            return;
        }

        if (key.startsWith(QStringLiteral("assets."))) {
            const QString& asset_key = key.mid(7); // len of "assets."
            const AssetType asset_type = pegasus_assets::type_by_suffix(asset_key);
            if (asset_type == AssetType::UNKNOWN) {
                on_error(lineno, tr_log("unknown asset type '%1', entry ignored").arg(asset_key));
                return;
            }

            add_asset(curr_game->assets, asset_type, val, dir_path);
            return;
        }

        if (!m_key_types.count(key)) {
            on_error(lineno, tr_log("unrecognized attribute name `%3`, ignored").arg(key));
            return;
        }
        switch (m_key_types.at(key)) {
            case MetaAttribType::TITLE:
                curr_game->title = val;
                break;
            case MetaAttribType::DEVELOPER:
                curr_game->developers.append(val);
                break;
            case MetaAttribType::PUBLISHER:
                curr_game->publishers.append(val);
                break;
            case MetaAttribType::GENRE:
                curr_game->genres.append(tokenize(val));
                break;
            case MetaAttribType::PLAYER_COUNT:
                {
                    const auto rx_match = m_player_regex.match(val);
                    if (rx_match.hasMatch()) {
                        const int a = rx_match.capturedRef(1).toInt();
                        const int b = rx_match.capturedRef(3).toInt();
                        curr_game->player_count = qMax(1, qMax(a, b));
                    }
                }
                break;
            case MetaAttribType::SHORT_DESC:
                curr_game->summary = val;
                break;
            case MetaAttribType::LONG_DESC:
                curr_game->description = val;
                break;
            case MetaAttribType::RELEASE:
                {
                    const auto rx_match = m_release_regex.match(val);
                    if (!rx_match.hasMatch()) {
                        on_error(lineno, tr_log("incorrect date format, should be YYYY(-MM(-DD))"));
                        return;
                    }

                    const int y = qMax(1, rx_match.captured(1).toInt());
                    const int m = qBound(1, rx_match.captured(3).toInt(), 12);
                    const int d = qBound(1, rx_match.captured(5).toInt(), 31);
                    curr_game->release_date = QDate(y, m, d);
                }
                break;
            case MetaAttribType::RATING:
                {
                    const auto rx_match_a = m_rating_percent_regex.match(val);
                    if (rx_match_a.hasMatch()) {
                        curr_game->rating = qBound(0.f, val.leftRef(val.length() - 1).toFloat() / 100.f, 1.f);
                        return;
                    }
                    const auto rx_match_b = m_rating_float_regex.match(val);
                    if (rx_match_b.hasMatch()) {
                        curr_game->rating = qBound(0.f, val.toFloat() / 100.f, 1.f);
                        return;
                    }
                    on_error(lineno, tr_log("failed to parse rating value"));
                }
                break;
            case MetaAttribType::LAUNCH_CMD:
                curr_game->launch_cmd = val;
                break;
        }
    };


    // the actual reading

    const QStringList possible_paths {
        dir_path + QStringLiteral("/metadata.pegasus.txt"),
        dir_path + QStringLiteral("/metadata.txt"),
    };
    for (const QString& path : possible_paths) {
        if (!::validFile(path))
            continue;

        qInfo().noquote() << tr_log("Found `%1`").arg(path);

        curr_game = nullptr;
        curr_config_path = path;
        config::readFile(path,  on_attribute, on_error);
        break; // if the first file exists, don't check the other
    }
}

} // namespace pegasus
} // namespace providers
