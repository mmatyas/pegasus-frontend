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


#include "CommandTokenizer.h"

#include <functional>


namespace {
int str_next_matching(const QString& str, const int from, const std::function<bool(const QChar)>& pred)
{
    for (int off = from; off < str.length(); off++) {
        if (pred(str.at(off)))
            return off;
    }
    return -1;
}
bool char_is_singlequote(const QChar c) {
    return c == QChar('\'');
}
bool char_is_doublequote(const QChar c) {
    return c == QChar('"');
}
bool char_is_nonspace(const QChar c) {
    return !c.isSpace();
}
bool char_is_space(const QChar c) {
    return c.isSpace();
}
} // namespace


namespace utils {
QStringList tokenize_command(const QString& str)
{
    QStringList results;
    int o_start = 0;
    int o_end = 0;

    while (o_start < str.length()) {
        o_start = str_next_matching(str, o_start, char_is_nonspace);
        if (o_start < 0)
            break;

        const QChar ch = str.at(o_start);
        if (char_is_singlequote(ch))
            o_end = str_next_matching(str, o_start + 1, char_is_singlequote) + 1;
        else if (char_is_doublequote(ch))
            o_end = str_next_matching(str, o_start + 1, char_is_doublequote) + 1;
        else
            o_end = str_next_matching(str, o_start + 1, char_is_space);

        if (o_end <= o_start)
            o_end = str.length();

        const int len = o_end - o_start;
        const bool is_quoted = len > 1
            && (ch == str.at(o_end - 1))
            && (char_is_singlequote(ch) || char_is_doublequote(ch));
        const int mid_from = is_quoted ? o_start + 1 : o_start;
        const int mid_len = is_quoted ? len - 2 : len;
        results.append(str.midRef(mid_from, mid_len).trimmed().toString());

        o_start = o_end;
    }
    return results;
}
} // namespace utils
