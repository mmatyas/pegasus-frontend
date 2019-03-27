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


class bench_ConfigFile : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void file();

private:
    std::vector<config::Entry> m_entries;

    void onAttributeFound(const config::Entry&);
    void onError(size_t, const QString&);

    void readStream(QTextStream&);
};


void bench_ConfigFile::onAttributeFound(const config::Entry& entry)
{
    m_entries.emplace_back(config::Entry {entry.line, entry.key, entry.values});
}

void bench_ConfigFile::onError(size_t linenum, const QString& msg)
{
    qWarning().noquote() << QObject::tr("line %1: %2")
        .arg(QString::number(linenum), msg);
}

void bench_ConfigFile::readStream(QTextStream& stream)
{
    config::read_stream(stream,
        [this](const config::Entry& entry){ this->onAttributeFound(entry); },
        [this](const config::Error& error){ this->onError(error.line, error.message); });
}


void bench_ConfigFile::empty()
{
    QByteArray buffer;
    QTextStream stream(buffer);

    QBENCHMARK {
        readStream(stream);
    }
}

void bench_ConfigFile::file()
{
    QBENCHMARK {
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("line 3: .*"));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("line 8: .*"));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("line 9: .*"));

        config::read_file(QStringLiteral(":/test.cfg"),
            [this](const config::Entry& entry){ this->onAttributeFound(entry); },
            [this](const config::Error& error){ this->onError(error.line, error.message); });
    }
}


QTEST_MAIN(bench_ConfigFile)
#include "bench_ConfigFile.moc"

