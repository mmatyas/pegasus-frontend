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


#include "ConfigFile.h"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QStringBuilder>


namespace config {

Config read(const QString& path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return {};

    QTextStream stream(&file);
    return readStream(stream, path);
}

Config readStream(QTextStream& stream, const QString& stream_name)
{
    static const QRegularExpression rx_section(R"(^\[(.*)\]$)"); // [name]
    static const QRegularExpression rx_keyval(R"(^([^=:]+)[=:](.+)$)"); // key = value
    static const QRegularExpression rx_multiline(R"(^\s+)"); // starts with whitespace

    Config config;
    ConfigGroup& cur_group = config[QString()];
    QString last_key;
    int linenum = 0;

    QString line;
    while (stream.readLineInto(&line)) {
        linenum++;
        const QStringRef trimmed_line = line.leftRef(-1).trimmed();

        // comment or empty line
        if (trimmed_line.isEmpty() || line.startsWith('#'))
            continue;

        // multiline (starts with whitespace but trimmed_line is not empty)
        if (rx_multiline.match(line).hasMatch() && !last_key.isEmpty()) {
            QVariant& last_item = cur_group[last_key];

            const QStringRef val = (trimmed_line == QLatin1String("."))
                ? QStringLiteral("\n").leftRef(-1)
                : trimmed_line;

            // array -> append to last
            if (last_item.type() == QVariant::List) {
                QVariantList list = last_item.toList();
                list.last() = QVariant(list.constLast().toString() + val);
                last_item = QVariant(list);
            }
            // regular value -> append
            else {
                last_item = QVariant(last_item.toString() + val);
            }

            continue;
        }

        // section (after the multiline check)
        const auto rx_section_match = rx_section.match(trimmed_line);
        if (rx_section_match.hasMatch()) {
            const QString group_name = rx_section_match.capturedRef(1).trimmed().toString();
            cur_group = config[group_name];
            last_key.clear();
            continue;
        }

        // keyval pair (after the multiline check)
        const auto rx_keyval_match = rx_keyval.match(trimmed_line);
        if (rx_keyval_match.hasMatch()) {
            const QString key = rx_keyval_match.capturedRef(1).trimmed().toString();
            if (key.isEmpty()) {
                qWarning().noquote()
                    << QObject::tr("`%1`, line %2: option name missing, line skipped")
                       .arg(stream_name, QString::number(linenum));
                continue;
            }
            const QString val = rx_keyval_match.capturedRef(2).trimmed().toString();
            if (val.isEmpty()) {
                qWarning().noquote()
                    << QObject::tr("`%1`, line %2: option value missing, line skipped")
                       .arg(stream_name, QString::number(linenum));
                continue;
            }

            if (cur_group.contains(key)) {
                QVariant& item = cur_group[key];

                // array -> append
                if (item.type() == QVariant::List) {
                    QVariantList list = item.toList();
                    list.append(val);
                    item = QVariant(list);
                }
                // regular value -> turn into array
                else {
                    QVariantList list = { item, val };
                    item = QVariant(list);
                }
            }
            else {
                cur_group.insert(key, val);
            }

            last_key = key;
            continue;
        }

        // invalid line
        qWarning().noquote() << QObject::tr("`%1`: line %2 is invalid, skipped")
                                .arg(stream_name, QString::number(linenum));
    }

    return config;
}

} // namespace config
