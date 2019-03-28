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


#include "MetaFile.h"

#include "LocaleUtils.h"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QTextStream>


namespace metafile {

void Entry::reset()
{
    line = 0;
    key.clear();
    values.clear();
}


/// Opens the file at the path, then calls the stream reading on it.
/// Returns false if the file could not be opened.
bool read_file(const QString& path,
               const std::function<void(const Entry&)>& onAttributeFound,
               const std::function<void(const Error&)>& onError)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    read_stream(stream, onAttributeFound, onError);
    return true;
}

/// Calls the stream reading on an already open, readable text file.
void read_file(QFile& file,
               const std::function<void(const Entry&)>& onAttributeFound,
               const std::function<void(const Error&)>& onError)
{
    Q_ASSERT(file.isOpen() && file.isReadable());
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    return read_stream(stream, onAttributeFound, onError);
}

/// Read and parse the stream, calling the callbacks when necessary.
void read_stream(QTextStream& stream,
                 const std::function<void(const Entry&)>& onAttributeFound,
                 const std::function<void(const Error&)>& onError)
{
    constexpr auto EMPTY_LINE_MARK = QChar('.');
    const QRegularExpression rx_keyval(QStringLiteral(R"(^([^:]+):(.*)$)")); // key: value

    Error error {0, {}};
    Entry entry {0, {}, {}};

    const auto close_current_attrib = [&](){
        if (!entry.key.isEmpty()) {
            if (entry.values.empty()) {
                onError({ entry.line, tr_log("attribute value missing, entry ignored") });
            }
            else
                onAttributeFound(entry);
        }

        entry.reset();
    };

    size_t linenum = 0;
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

        // multiline (starts with whitespace but also has content)
        if (line.at(0).isSpace()) {
            if (entry.key.isEmpty()) {
                onError({ linenum, tr_log("line starts with whitespace, but no attribute has been defined yet") });
                continue;
            }

            if (trimmed_line == EMPTY_LINE_MARK) {
                entry.values.emplace_back(QString());
                continue;
            }

            entry.values.emplace_back(trimmed_line.toString());
            continue;
        }

        // either a new entry or error - in both cases, the previous entry should be closed
        close_current_attrib();

        // empty line, skip
        if (trimmed_line.isEmpty())
            continue;

        // keyval pair (after the multiline check)
        const auto rx_keyval_match = rx_keyval.match(trimmed_line);
        if (rx_keyval_match.hasMatch()) {
            // the key is never empty if the regex matches the *trimmed* line
            entry.key = rx_keyval_match.capturedRef(1).trimmed().toString().toLower();

            // the value can be empty here, if it's purely multiline
            auto value_part = rx_keyval_match.capturedRef(2).trimmed();
            if (!value_part.isEmpty())
                entry.values.emplace_back(value_part.toString());

            entry.line = linenum;
            continue;
        }

        // invalid line
        onError({ linenum, tr_log("line invalid, skipped") });
    }

    // the very last line
    linenum++;
    close_current_attrib();
}


/// Creates a single text from the separate lines. Lines are expected to be
/// null strings or non-empty trimmed text
QString merge_lines(const std::vector<QString>& lines)
{
    if (lines.empty())
        return QString();


    constexpr QChar SPACE(' ');
    constexpr QChar NEWLINE('\n');

    int len = 0;
    for (const QString& line : lines)
        len += line.length() + 1; // +1 for likely space

    QString out;
    out.reserve(len);


    for (const QString& line : lines) {
        if (line.isNull()) {
            out += QStringLiteral("\n\n");
            continue;
        }

        if (!out.endsWith(NEWLINE))
            out += SPACE;

        out += line;
    }

    return out.trimmed();
}

} // namespace metafile
