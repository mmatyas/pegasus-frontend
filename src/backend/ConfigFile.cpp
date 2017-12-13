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
    static const QRegularExpression rx_section(R"(^\[(.*)\]\s*$)"); // [name]
    static const QRegularExpression rx_keyval(R"(^([^=]+)=(.*)$)"); // key = value
    static const QRegularExpression rx_multiline(R"(^\s+)"); // starts with whitespace

    Config config;
    QString current_group;
    QString current_key;
    bool current_key_is_array(false);
    int linenum = 0;

    QString line;
    while (stream.readLineInto(&line)) {
        linenum++;

        // comment or empty line
        if (line.startsWith('#') || line.leftRef(-1).trimmed().isEmpty())
            continue;

        // section
        const auto rx_section_match = rx_section.match(line);
        if (rx_section_match.hasMatch()) {
            current_group = rx_section_match.capturedRef(1).trimmed().toString() % '.';
            current_key.clear();
            current_key_is_array = false;
            continue;
        }

        // keyval pair
        const auto rx_keyval_match = rx_keyval.match(line);
        if (rx_keyval_match.hasMatch()) {
            const QStringRef keyref = rx_keyval_match.capturedRef(1).trimmed();
            const bool key_is_array = keyref.endsWith(QLatin1String("[]"));
            const QString key = key_is_array
                ? current_group % keyref.left(keyref.length() - 2).trimmed().toString()
                : current_group % keyref.toString();
            const QString val = rx_keyval_match.capturedRef(2).trimmed().toString();

            if (key_is_array) {
                if (!config.contains(key) || !config[key].canConvert(QVariant::List)) {
                    config.insert(key, QVariant(QVariant::List));
                }
                config[key].toList().append(val);
            }
            else {
                config[key] = QVariant(val);
            }

            current_key = key;
            current_key_is_array = key_is_array;
            continue;
        }

        // multiline
        if (rx_multiline.match(line).hasMatch() && !current_key.isEmpty()) {
            QString val = line.trimmed();
            if (val.length() == 1 && val.startsWith('.'))
                val = '\n';

            if (current_key_is_array) {
                QList<QVariant> list(config[current_key].toList());
                list.last() = QVariant(list.constLast().toString() + val);
                config[current_key] = QVariant(list);
            }
            else {
                config[current_key] = QVariant(config[current_key].toString() + val);
            }

            continue;
        }

        // invalid line
        qWarning().noquote() << QObject::tr("`%1`: line %2 is invalid, skipped")
                                .arg(stream_name, QString::number(linenum));
    }

    return config;
}

} // namespace config
