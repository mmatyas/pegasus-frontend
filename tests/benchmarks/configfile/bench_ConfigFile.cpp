// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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


class bench_ConfigFile : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void file();

private:
    QHash<QString, QHash<QString, QString>> m_config;
    QString m_current_section;

    void onSectionFound(const QString);
    void onAttributeFound(const QString, const QString);
    void onError(const int, const QString);

    void readStream(QTextStream&);
};


void bench_ConfigFile::onSectionFound(const QString section_name)
{
    m_current_section = section_name;
    m_config[m_current_section];
}

void bench_ConfigFile::onAttributeFound(const QString key, const QString val)
{
    QHash<QString, QString>& section = m_config[m_current_section];
    if (section.count(key))
        section[key] += QStringLiteral(", ") % val;
    else
        section.insert(key, val);
}

void bench_ConfigFile::onError(const int linenum, const QString msg)
{
    qWarning().noquote() << QObject::tr("line %1: %2")
        .arg(QString::number(linenum), msg);
}

void bench_ConfigFile::readStream(QTextStream& stream)
{
    config::readStream(stream,
        [this](const int, const QString name){ this->onSectionFound(name); },
        [this](const int, const QString key, const QString val){ this->onAttributeFound(key, val); },
        [this](const int linenum, const QString msg){ this->onError(linenum, msg); });
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
        QTest::ignoreMessage(QtWarningMsg, "line 3: line invalid, skipped");
        QTest::ignoreMessage(QtWarningMsg, "line 14: line invalid, skipped");
        QTest::ignoreMessage(QtWarningMsg, "line 16: line invalid, skipped");

        config::readFile(":/test.cfg",
            [this](const int, const QString name){ this->onSectionFound(name); },
            [this](const int, const QString key, const QString val){ this->onAttributeFound(key, val); },
            [this](const int linenum, const QString msg){ this->onError(linenum, msg); });
    }
}


QTEST_MAIN(bench_ConfigFile)
#include "bench_ConfigFile.moc"

