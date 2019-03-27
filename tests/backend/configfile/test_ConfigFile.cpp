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


#include <QtTest/QtTest>

#include "parsers/ConfigFile.h"

#include <tuple>
#include <vector>


class test_ConfigFile : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void datablob();
    void file();

    void merge_lines();
    void merge_lines_data();

private:
    std::vector<config::Entry> m_entries;

    void onAttributeFound(const config::Entry&);
    void onError(const config::Error&);

    void readStream(QTextStream&);
};


void test_ConfigFile::onAttributeFound(const config::Entry& entry)
{
    m_entries.emplace_back(entry);
}

void test_ConfigFile::onError(const config::Error& error)
{
    qWarning().noquote() << QObject::tr("line %1: %2")
        .arg(QString::number(error.line), error.message);
}

void test_ConfigFile::readStream(QTextStream& stream)
{
    config::readStream(stream,
        [this](const config::Entry& entry){ this->onAttributeFound(entry); },
        [this](const config::Error& error){ this->onError(error); });
}


void test_ConfigFile::empty()
{
    QByteArray buffer;
    QTextStream stream(buffer);

    readStream(stream);

    QCOMPARE(m_entries.empty(), true);
}

void test_ConfigFile::datablob()
{
    QByteArray buffer(1024, 0x0);
    QTextStream stream(buffer);

    QTest::ignoreMessage(QtWarningMsg, "line 1: line invalid, skipped");
    readStream(stream);

    QCOMPARE(m_entries.empty(), true);
}

void test_ConfigFile::file()
{
    QTest::ignoreMessage(QtWarningMsg, "line 3: line starts with whitespace, but no attribute has been defined yet");
    QTest::ignoreMessage(QtWarningMsg, "line 8: attribute value missing, entry ignored");
    QTest::ignoreMessage(QtWarningMsg, "line 9: line invalid, skipped");
    QTest::ignoreMessage(QtWarningMsg, "line 23: line starts with whitespace, but no attribute has been defined yet");
    config::readFile(":/test.cfg",
        [this](const config::Entry& entry){ this->onAttributeFound(entry); },
        [this](const config::Error& error){ this->onError(error); });

    const decltype(m_entries) expected {
        config::Entry { 5, "key1", {"val"} },
        config::Entry { 6, "key2", {"val"} },
        config::Entry { 7, "key with spaces", {"val with spaces"} },
        config::Entry { 11, "multiline1", {"hello", "world!"} },
        config::Entry { 13, "multiline2", {"purely", "multiline"} },
        config::Entry { 16, "multiline3", {"text", "with", QString(), "line break"} },
        config::Entry { 20, "multiline4", {"text", "stops here"} },
        config::Entry { 25, "list1", {"list", "of", "items"} },
    };


    const size_t count = std::min(m_entries.size(), expected.size());
    for (size_t i = 0; i < count; i++) {
        QCOMPARE(m_entries.at(i).line, expected.at(i).line);
        QCOMPARE(m_entries.at(i).key, expected.at(i).key);
        QCOMPARE(m_entries.at(i).values, expected.at(i).values);
    }
    QCOMPARE(m_entries.size(), expected.size());
}

void test_ConfigFile::merge_lines()
{
    QFETCH(QStringList, parts);
    QFETCH(QString, expected);

    const QVector<QString> parts_vec = parts.toVector();

    QCOMPARE(config::mergeLines(parts_vec), expected);
}

void test_ConfigFile::merge_lines_data()
{
    QTest::addColumn<QStringList>("parts");
    QTest::addColumn<QString>("expected");

    QTest::newRow("null") << QStringList() << QString();
    QTest::newRow("simple") << QStringList({ "aa", "bb" }) << QStringLiteral("aa bb");
    QTest::newRow("empty lines") << QStringList({ QString(), "aa", QString(), "bb", QString() }) << QStringLiteral("aa\n\nbb");
}


QTEST_MAIN(test_ConfigFile)
#include "test_ConfigFile.moc"

