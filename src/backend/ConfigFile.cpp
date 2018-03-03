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

void readFile(const QString& path,
              const std::function<void(const int, const QString)>& onSectionFound,
              const std::function<void(const int, const QString, const QString)>& onAttributeFound,
              const std::function<void(const int, const QString)>& onError)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return;

    QTextStream stream(&file);
    return readStream(stream, onSectionFound, onAttributeFound, onError);
}

void readStream(QTextStream& stream,
                const std::function<void(const int, const QString)>& onSectionFound,
                const std::function<void(const int, const QString, const QString)>& onAttributeFound,
                const std::function<void(const int, const QString)>& onError)
{
    static const QRegularExpression rx_section(R"(^\[(.*)\]$)"); // [name]
    static const QRegularExpression rx_keyval(R"(^([^=:]+)[=:](.*)$)"); // key = value
    static const QRegularExpression rx_multiline(R"(^\s+)"); // starts with whitespace

    QString last_key;
    QString last_val;
    int linenum = 0;

    const auto on_attrib_complete = [&](){
        if (!last_key.isEmpty() && !last_val.isEmpty())
            onAttributeFound(linenum, last_key, last_val);

        last_key.clear();
        last_val.clear();
    };

    QString line;
    while (stream.readLineInto(&line)) {
        linenum++;
        const QStringRef trimmed_line = line.leftRef(-1).trimmed();

        // comment or empty line
        if (trimmed_line.isEmpty() || line.startsWith('#'))
            continue;

        // multiline (starts with whitespace but trimmed_line is not empty)
        if (rx_multiline.match(line).hasMatch() && !last_key.isEmpty()) {
            if (trimmed_line == QLatin1String(".")) {
                last_val.append('\n');
                continue;
            }
            if (!last_val.isEmpty() && !last_val.endsWith('\n')) {
                last_val.append(' ');
            }
            last_val.append(trimmed_line);
            continue;
        }

        // section (after the multiline check)
        const auto rx_section_match = rx_section.match(trimmed_line);
        if (rx_section_match.hasMatch()) {
            on_attrib_complete();

            const QString group_name = rx_section_match.capturedRef(1).trimmed().toString();
            onSectionFound(linenum, group_name);
            continue;
        }

        // keyval pair (after the multiline check)
        const auto rx_keyval_match = rx_keyval.match(trimmed_line);
        if (rx_keyval_match.hasMatch()) {
            on_attrib_complete();

            const QString key = rx_keyval_match.capturedRef(1).trimmed().toString();
            if (key.isEmpty()) {
                onError(linenum, QStringLiteral("attribute name missing, line skipped"));
                continue;
            }
            last_key = key;
            // the value can be empty here, in case it's purely multiline
            last_val = rx_keyval_match.capturedRef(2).trimmed().toString();
            continue;
        }

        // invalid line
        onError(linenum, "line invalid, skipped");
    }

    // the very last line
    if (!last_key.isEmpty() && last_val.isEmpty())
        onError(linenum, QStringLiteral("attribute value missing, line skipped"));
    else
        on_attrib_complete();
}

} // namespace config
