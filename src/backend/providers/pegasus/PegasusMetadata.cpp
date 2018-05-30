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
#include "PegasusAssets.h"
#include "PegasusCommon.h"
#include "Utils.h"
#include "model/gaming/Game.h"

#include <QDebug>
#include <QFileInfo>
#include <QStringBuilder>


namespace {

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
                                      const QHash<QString, model::Game*>& games,
                                      const QHash<QString, model::Collection*>&) const
{
    pegasus_assets::findAssets(dir_list, games);

    for (const QString& dir_path : dir_list)
        read_metadata_file(dir_path, games);
}


void PegasusMetadata::read_metadata_file(const QString& dir_path,
                                         const QHash<QString, model::Game*>& games) const
{
    QString curr_config_path;
    model::Game* curr_game = nullptr;

    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << tr_log("`%1`, line %2: %3").arg(curr_config_path, QString::number(lineno), msg);
    };
    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        if (key == QLatin1String("file")) {
            const QString curr_game_path = dir_path % '/' % val;
            const QFileInfo fileinfo(curr_game_path);
            curr_game = nullptr;

            if (!games.contains(fileinfo.canonicalFilePath())) {
                on_error(lineno,
                    tr_log("the game `%1` is either missing or excluded, values for it will be ignored").arg(val));
                return;
            }

            curr_game = games[fileinfo.canonicalFilePath()];
            Q_ASSERT(curr_game);
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
        if (!m_key_types.contains(key)) {
            on_error(lineno, tr_log("unrecognized attribute name `%3`, ignored").arg(key));
            return;
        }

        switch (m_key_types[key]) {
            case MetaAttribType::TITLE:
                curr_game->m_title = val;
                break;
            case MetaAttribType::DEVELOPER:
                curr_game->addDeveloper(val);
                break;
            case MetaAttribType::PUBLISHER:
                curr_game->addPublisher(val);
                break;
            case MetaAttribType::GENRE:
                curr_game->addGenres(tokenize(val));
                break;
            case MetaAttribType::PLAYER_COUNT:
                {
                    const auto rx_match = m_player_regex.match(val);
                    if (rx_match.hasMatch()) {
                        const int a = rx_match.capturedRef(1).toInt();
                        const int b = rx_match.capturedRef(3).toInt();
                        curr_game->m_players = qMax(1, qMax(a, b));
                    }
                }
                break;
            case MetaAttribType::SHORT_DESC:
                curr_game->m_summary = val;
                break;
            case MetaAttribType::LONG_DESC:
                curr_game->m_description = val;
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
                    curr_game->setRelease(QDate(y, m, d));
                }
                break;
            case MetaAttribType::RATING:
                {
                    const auto rx_match_a = m_rating_percent_regex.match(val);
                    if (rx_match_a.hasMatch()) {
                        curr_game->m_rating = qBound(0.f, val.leftRef(val.length() - 1).toFloat() / 100.f, 1.f);
                        return;
                    }
                    const auto rx_match_b = m_rating_float_regex.match(val);
                    if (rx_match_b.hasMatch()) {
                        curr_game->m_rating = qBound(0.f, val.toFloat() / 100.f, 1.f);
                        return;
                    }
                    on_error(lineno, tr_log("failed to parse rating value"));
                }
                break;
            case MetaAttribType::LAUNCH_CMD:
                curr_game->m_launch_cmd = val;
                break;
        }
    };


    // the actual reading

    curr_config_path = dir_path + QStringLiteral("/metadata.pegasus.txt");
    if (::validFile(curr_config_path)) {
        qInfo().noquote() << tr_log("Found `%1`").arg(curr_config_path);
        config::readFile(curr_config_path,  on_attribute, on_error);
    }
    else {
        curr_config_path = dir_path + QStringLiteral("/metadata.txt");
        curr_game = nullptr;
        if (::validFile(curr_config_path)) {
            qInfo().noquote() << tr_log("Found `%1`").arg(curr_config_path);
            config::readFile(curr_config_path,  on_attribute, on_error);
        }
    }
}

} // namespace pegasus
} // namespace providers
