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


#pragma once

#include "types/KeyEventType.h"
#include "utils/HashMap.h"
#include "utils/StrBoolConverter.h"

#include <QString>
#include <QStringList>
#include <QTextStream>
#include <map>


namespace appsettings {

enum class ConfigEntryCategory : unsigned char {
    GENERAL,
    PROVIDERS,
    KEYS,
};

enum class ConfigEntryGeneralOption : unsigned char {
    FULLSCREEN,
    MOUSE_SUPPORT,
    LOCALE,
    THEME,
};

struct ConfigEntryMaps {
    ConfigEntryMaps();

    using Category = ConfigEntryCategory;
    using GeneralOption = ConfigEntryGeneralOption;

    const HashMap<QString, Category> str_to_category;
    const HashMap<QString, GeneralOption> str_to_general_opt;
    const HashMap<QString, KeyEvent> str_to_key_opt;
};


class SettingsFileContext {
protected:
    SettingsFileContext();

    const QString config_path;
    const ConfigEntryMaps maps;
};


class LoadContext : public SettingsFileContext {
public:
    LoadContext();

    void load() const;

private:
    void log_error(const size_t lineno, const QString& msg) const;
    void log_unknown_key(const size_t lineno, const QString& key) const;
    void log_needs_bool(const size_t lineno, const QString& key) const;

private:
    void handle_entry(const size_t lineno, const QString& key, const std::vector<QString>& vals) const;
    void handle_general_attrib(const size_t lineno, const QString& key, const QString& val,
                               QStringList& sections) const;
    void handle_provider_attrib(const size_t lineno, const QString& key,
                                const std::vector<QString>& vals,
                                QStringList& sections) const;
    void handle_key_attrib(const size_t lineno, const QString& key, const QString& val,
                           QStringList& sections) const;

private:
    const StrBoolConverter strconv;
    const std::map<QString, QKeySequence> reverse_gamepadButtonNames;
};


class SaveContext : public SettingsFileContext {
public:
    SaveContext();

    void save() const;

private:
    void print_general(QTextStream& stream) const;
    void print_providers(QTextStream& stream) const;
    void print_keys(QTextStream& stream) const;

private:
    using CategoryStrMap = HashMap<ConfigEntryCategory, QString, EnumHash>;

    const QString STR_TRUE;
    const QString STR_FALSE;
    const QString LINE_TEMPLATE;

    const CategoryStrMap category_names;

    CategoryStrMap gen_category_names() const;
};

} // namespace appsettings
