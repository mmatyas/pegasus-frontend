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
              const std::function<void(const int, const QString, const QString)>& onAttributeFound,
              const std::function<void(const int, const QString)>& onError)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return;

    QTextStream stream(&file);
    return readStream(stream, onAttributeFound, onError);
}

void readStream(QTextStream& stream,
                const std::function<void(const int, const QString, const QString)>& onAttributeFound,
                const std::function<void(const int, const QString)>& onError)
{
    const QRegularExpression rx_keyval(R"(^([^:]+):(.*)$)"); // key = value

    QString last_key;
    QString last_val;
    int linenum = 0;

    const auto close_current_attrib = [&](){
        // linenum is decrased by 1 because this function
        // is called in the line *after* the attribute ends
        if (!last_key.isEmpty()) {
            if (last_val.isEmpty())
                onError(linenum - 1, QStringLiteral("attribute value missing, entry ignored"));
            else
                onAttributeFound(linenum - 1, last_key, last_val);
        }

        last_key.clear();
        last_val.clear();
    };

    QString line;
    while (stream.readLineInto(&line)) {
        linenum++;

        if (line.startsWith('#'))
            continue;

        const QStringRef trimmed_line = line.leftRef(-1).trimmed();
        if (trimmed_line.isEmpty()) {
            close_current_attrib();
            continue;
        }

        // multiline (starts with whitespace but trimmed_line is not empty)
        if (line.at(0).isSpace()) {
            if (last_key.isEmpty()) {
                onError(linenum, QStringLiteral("multiline value found, but no attribute has been defined yet"));
                continue;
            }
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

        // either a new entry or error - in both cases, the previous entry should be closed
        close_current_attrib();

        // keyval pair (after the multiline check)
        const auto rx_keyval_match = rx_keyval.match(trimmed_line);
        if (rx_keyval_match.hasMatch()) {
            // the key is never empty if the regex matches the *trimmed* line
            last_key = rx_keyval_match.capturedRef(1).trimmed().toString().toLower();
            Q_ASSERT(!last_key.isEmpty());
            // the value can be empty here, if it's purely multiline
            last_val = rx_keyval_match.capturedRef(2).trimmed().toString();
            continue;
        }

        // invalid line
        onError(linenum, QStringLiteral("line invalid, skipped"));
    }

    // the very last line
    linenum++;
    close_current_attrib();
}

} // namespace config
