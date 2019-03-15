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


namespace {
int str_next_nonspace(const QString& str, int from = 0)
{
    for (int off = from; off < str.length(); off++) {
        if (!str.at(off).isSpace())
            return off;
    }
    return -1;
}
int str_next_space(const QString& str, int from = 0)
{
    for (int off = from; off < str.length(); off++) {
        if (str.at(off).isSpace())
            return off;
    }
    return -1;
}
int str_next_matching(const QString& str, const QChar kar, int from = 0)
{
    for (int off = from; off < str.length(); off++) {
        if (str.at(off) == kar)
            return off;
    }
    return -1;
}
} // namespace


namespace utils {
QStringList tokenize_command(const QString& str)
{
    constexpr QChar SINGLE_QUOTE = '\'';
    constexpr QChar DOUBLE_QUOTE = '"';

    QStringList results;
    int o_start = 0;
    int o_end = 0;

    while (o_start < str.length()) {
        o_start = str_next_nonspace(str, o_start);
        if (o_start < 0)
            break;

        const QChar ch = str.at(o_start);
        if (ch == SINGLE_QUOTE || ch == DOUBLE_QUOTE)
            o_end = str_next_matching(str, ch, o_start + 1) + 1;

        if (o_end <= o_start)
            o_end = str_next_space(str, o_start + 1);
        if (o_end <= o_start)
            o_end = str.length();

        if (o_start < o_end) {
            const int len = o_end - o_start;
            const bool is_quoted = len > 1
                && (str.at(o_start) == str.at(o_end - 1))
                && (str.at(o_start) == SINGLE_QUOTE || str.at(o_start) == DOUBLE_QUOTE);

            const int mid_from = is_quoted ? o_start + 1 : o_start;
            const int mid_len = is_quoted ? len - 2 : len;
            results.append(str.mid(mid_from, mid_len));
        }

        o_start = o_end;
    }
    return results;
}
} // namespace utils
