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


class test_ConfigFile : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void datablob();
    void file();

private:
    QHash<QString, QHash<QString, QString>> m_config;
    QString m_current_section;

    void onSectionFound(const QString);
    void onAttributeFound(const QString, const QString);
    void onError(const int, const QString);

    void readStream(QTextStream&);
};


void test_ConfigFile::onSectionFound(const QString section_name)
{
    m_current_section = section_name;
    m_config[m_current_section];
}

void test_ConfigFile::onAttributeFound(const QString key, const QString val)
{
    QHash<QString, QString>& section = m_config[m_current_section];
    if (section.count(key))
        section[key] += QStringLiteral(", ") % val;
    else
        section.insert(key, val);
}

void test_ConfigFile::onError(const int linenum, const QString msg)
{
    qWarning().noquote() << QObject::tr("line %1: %2")
        .arg(QString::number(linenum), msg);
}

void test_ConfigFile::readStream(QTextStream& stream)
{
    config::readStream(stream,
        [this](const int, const QString name){ this->onSectionFound(name); },
        [this](const int, const QString key, const QString val){ this->onAttributeFound(key, val); },
        [this](const int linenum, const QString msg){ this->onError(linenum, msg); });
}


void test_ConfigFile::empty()
{
    QByteArray buffer;
    QTextStream stream(buffer);

    readStream(stream);

    QCOMPARE(m_config.isEmpty(), true);
}

void test_ConfigFile::datablob()
{
    QByteArray buffer(1024, 0x0);
    QTextStream stream(buffer);

    QTest::ignoreMessage(QtWarningMsg, "line 1: line invalid, skipped");
    readStream(stream);

    QCOMPARE(m_config.isEmpty(), true);
}

void test_ConfigFile::file()
{
    QTest::ignoreMessage(QtWarningMsg, "line 3: multiline value found, but no attribute has been defined yet");
    QTest::ignoreMessage(QtWarningMsg, "line 14: multiline value found, but no attribute has been defined yet");
    QTest::ignoreMessage(QtWarningMsg, "line 16: line invalid, skipped");
    config::readFile(":/test.cfg",
        [this](const int, const QString name){ this->onSectionFound(name); },
        [this](const int, const QString key, const QString val){ this->onAttributeFound(key, val); },
        [this](const int linenum, const QString msg){ this->onError(linenum, msg); });

    QHash<QString, QHash<QString, QString>> expected {
        {QString(), { {"global", "value"} }},
        {"sectionname", {
              {"key1", "val"},
              {"key2", "val"},
              {"key with spaces", "val with spaces"},
              {"option with", "as delimiter"},
        }},
        {"sectionname with spaces", {}},
        {"multilines", {
             {"multiline1", "hello world!"},
             {"multiline2", "a line with\nline break"},
             {"multiline3", "purely multiline"},
        }},
        {"arrays", {
             {"array", "value1, value2, value3"}
        }},
    };
    QCOMPARE(m_config, expected);
}


QTEST_MAIN(test_ConfigFile)
#include "test_ConfigFile.moc"

