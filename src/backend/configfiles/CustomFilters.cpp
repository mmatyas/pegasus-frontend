// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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


#include "CustomFilters.h"

#include "ConfigFile.h"
#include "LocaleUtils.h"
#include "Paths.h"
#include "model/Filters.h"
#include "model/gaming/Game.h"
#include "utils/HashMap.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>


namespace {

QString default_config_path()
{
    return paths::writableConfigDir() + QStringLiteral("/filters.txt");
}

QVector<model::Filter*> default_filters()
{
    QVector<model::Filter*> filters {
        new model::Filter(QStringLiteral("Favorites")),
        new model::Filter(QStringLiteral("Multiplayer")),
    };
    filters[0]->rulesMut().append(model::FilterRule {
        QStringLiteral("favorite"),
        model::FilterRuleType::IS_TRUE,
        QRegularExpression(),
    });
    filters[1]->rulesMut().append(model::FilterRule {
        QStringLiteral("players"),
        model::FilterRuleType::NOT_EQUALS,
        QRegularExpression(QStringLiteral("1")),
    });
    return filters;
}

} // namespace


QVector<model::Filter*> CustomFilters::read(const QString& path)
{
    const QString config_path = path.isEmpty() ? default_config_path() : path;
    if (!QFileInfo::exists(config_path))
        return default_filters();

    QFile db_file(config_path);
    if (!db_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << tr_log("Could not open `%1` for reading, custom filters are not loaded.")
                      .arg(config_path);
        return default_filters();
    }


    HashMap<QString, model::Filter*> filters;
    model::Filter* current_filter = nullptr;

    const QRegularExpression rx_rule(QStringLiteral(R"(^([a-zA-Z\.]+) +([a-z_]+)( +.+)?$)"));
    const HashMap<QString, model::FilterRuleType> possible_comparisons {
        { QStringLiteral("is_true"), model::FilterRuleType::IS_TRUE },
        { QStringLiteral("is_false"), model::FilterRuleType::IS_FALSE },
        { QStringLiteral("empty"), model::FilterRuleType::EMPTY },
        { QStringLiteral("not_empty"), model::FilterRuleType::NOT_EMPTY },
        { QStringLiteral("contains"), model::FilterRuleType::CONTAINS },
        { QStringLiteral("not_contains"), model::FilterRuleType::NOT_CONTAINS },
        { QStringLiteral("equals"), model::FilterRuleType::EQUALS },
        { QStringLiteral("not_equals"), model::FilterRuleType::NOT_EQUALS },
    };


    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << tr_log("`%1`, line %2: %3").arg(config_path, QString::number(lineno), msg);
    };
    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        if (key == QLatin1String("filter")) {
            if (!filters.count(val))
                filters.emplace(val, new model::Filter(val));

            current_filter = filters.at(val);
            Q_ASSERT(current_filter);
            return;
        }
        if (!current_filter) {
            on_error(lineno, tr_log("no filter defined yet, entry ignored"));
            return;
        }
        if (key.startsWith(QLatin1String("x-"))) {
            // TODO: unimplemented
            return;
        }
        if (key != QLatin1String("rule")) {
            on_error(lineno, tr_log("unrecognized attribute name `%1`, ignored").arg(key));
            return;
        }

        const auto rx_rule_match = rx_rule.match(val);
        if (!rx_rule_match.hasMatch()) {
            on_error(lineno, tr_log("rule should be in 'property comparison' or 'property comparison regex' format"));
            return;
        }

        const QString property_str = rx_rule_match.captured(1);
        Q_ASSERT(!property_str.isEmpty());
        const bool prop_valid = (0 <= model::Game::staticMetaObject.indexOfProperty(property_str.toLatin1().data()));
        if (!prop_valid) {
            on_error(lineno, tr_log("unrecognized game property `%1`, rule ignored").arg(property_str));
            return;
        }

        const QString comparison_str = rx_rule_match.captured(2);
        Q_ASSERT(!comparison_str.isEmpty());
        const bool comparison_valid = possible_comparisons.count(comparison_str);
        if (!comparison_valid) {
            on_error(lineno, tr_log("unrecognized comparison `%1`, rule ignored").arg(comparison_str));
            return;
        }

        const model::FilterRuleType comparison = possible_comparisons.at(comparison_str);
        QString comparison_arg;

        switch (comparison) {
            case model::FilterRuleType::CONTAINS:
            case model::FilterRuleType::NOT_CONTAINS:
            case model::FilterRuleType::EQUALS:
            case model::FilterRuleType::NOT_EQUALS: {
                QStringRef comp_ref = rx_rule_match.capturedRef(3);
                if (!comp_ref.isNull()) {
                    comp_ref = comp_ref.trimmed();
                    if (comp_ref.startsWith('"') && comp_ref.endsWith('"'))
                        comp_ref = comp_ref.mid(1, comp_ref.length() - 2);
                }
                if (comp_ref.isEmpty()) {
                    on_error(lineno, tr_log("comparison type `%1` requires a third parameter, rule ignored").arg(property_str));
                    return;
                }
                comparison_arg = comp_ref.toString();
                break;
            }
            default:
                break;
        }

        current_filter->rulesMut().append(model::FilterRule {
            property_str,
            comparison,
            QRegularExpression(comparison_arg),
        });
    };

    // the actual reading
    config::readFile(db_file, on_attribute, on_error);


    QVector<model::Filter*> out;
    for (const auto& entry : filters) {
        if (!entry.second->rules().isEmpty())
            out.push_back(entry.second);
    }
    qInfo() << tr_log("Found %1 custom filters").arg(out.length());
    return out;
}
