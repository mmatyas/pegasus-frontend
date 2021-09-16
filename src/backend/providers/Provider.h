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


#pragma once

#include "utils/HashMap.h"

#include <QFileInfo>
#include <QString>
#include <QObject>
#include <vector>

namespace model { class Collection; }
namespace model { class Game; }
namespace model { class GameFile; }
namespace providers { class SearchContext; }


namespace providers {

constexpr uint8_t PROVIDER_FLAG_NONE = 0;
constexpr uint8_t PROVIDER_FLAG_INTERNAL = (1 << 0);
constexpr uint8_t PROVIDER_FLAG_HIDE_PROGRESS = (1 << 1);


class Provider : public QObject {
    Q_OBJECT

public:
    explicit Provider(QLatin1String codename, QString display_name, uint8_t flags, QObject* parent = nullptr);
    explicit Provider(QLatin1String codename, QString display_name, QObject* parent = nullptr);
    virtual ~Provider();

    bool enabled() const { return m_enabled; }
    Provider& setEnabled(bool);

    virtual Provider& run(SearchContext&) { return *this; }

    // events
    virtual void onGameFavoriteChanged(const QVector<model::Game*>&) {}
    virtual void onGameLaunched(model::GameFile* const) {}
    virtual void onGameFinished(model::GameFile* const) {}

    // common
    const QLatin1String& codename() const { return m_codename; }
    const QString& display_name() const { return m_display_name; }
    uint8_t flags() const { return m_flags; }

    Provider& setOption(const QString&, QString);
    Provider& setOption(const QString&, std::vector<QString>);
    const HashMap<QString, std::vector<QString>>& options() const { return m_options; }

signals:
    void progressChanged(float);

private:
    const QLatin1String m_codename;
    const QString m_display_name;
    const uint8_t m_flags;

    bool m_enabled;
    HashMap<QString, std::vector<QString>> m_options;
};

} // namespace providers
