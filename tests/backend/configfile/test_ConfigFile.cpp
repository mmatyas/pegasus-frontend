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


#include <QtTest/QtTest>

#include "ConfigFile.h"

#include <tuple>
#include <vector>


class test_ConfigFile : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void datablob();
    void file();

private:
    std::vector<std::tuple<int, QString, QString>> m_entries;

    void onAttributeFound(int, const QString&, const QString&);
    void onError(int, const QString&);

    void readStream(QTextStream&);
};


void test_ConfigFile::onAttributeFound(int line, const QString& key, const QString& val)
{
    m_entries.emplace_back(line, key, val);
}

void test_ConfigFile::onError(int linenum, const QString& msg)
{
    qWarning().noquote() << QObject::tr("line %1: %2")
        .arg(QString::number(linenum), msg);
}

void test_ConfigFile::readStream(QTextStream& stream)
{
    config::readStream(stream,
        [this](const int lineno, const QString key, const QString val){ this->onAttributeFound(lineno, key, val); },
        [this](const int lineno, const QString msg){ this->onError(lineno, msg); });
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
    QTest::ignoreMessage(QtWarningMsg, "line 3: multiline value found, but no attribute has been defined yet");
    QTest::ignoreMessage(QtWarningMsg, "line 8: attribute value missing, entry ignored");
    QTest::ignoreMessage(QtWarningMsg, "line 9: line invalid, skipped");
    config::readFile(":/test.cfg",
        [this](const int lineno, const QString key, const QString val){ this->onAttributeFound(lineno, key, val); },
        [this](const int lineno, const QString msg){ this->onError(lineno, msg); });

    const decltype(m_entries) expected {
        std::make_tuple(5, QStringLiteral("key1"), QStringLiteral("val")),
        std::make_tuple(6, QStringLiteral("key2"), QStringLiteral("val")),
        std::make_tuple(7, QStringLiteral("key with spaces"), QStringLiteral("val with spaces")),
        std::make_tuple(11, QStringLiteral("multiline1"), QStringLiteral("hello world!")),
        std::make_tuple(13, QStringLiteral("multiline2"), QStringLiteral("a line with\nline break")),
        std::make_tuple(17, QStringLiteral("multiline3"), QStringLiteral("purely multiline")),
    };
    /*for (const auto& entry : m_entries) {
        qDebug() << "line" << std::get<0>(entry) << "key" << std::get<1>(entry) << "val" << std::get<2>(entry);
    }*/
    QCOMPARE(m_entries, expected);
}


QTEST_MAIN(test_ConfigFile)
#include "test_ConfigFile.moc"

