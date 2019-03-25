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

#include "ProcessLauncher.h"


namespace {
QString fallback_workdir() {
#ifdef Q_OS_WIN
    return QStringLiteral("c:\\fallback\\path");
#else
    return QStringLiteral("/fallback/path");
#endif
}
} // namespace


class test_ProcessLauncher : public QObject {
    Q_OBJECT

private slots:
    void exe_path();
    void exe_path_data();

    void workdir_path();
    void workdir_path_data();
};

void test_ProcessLauncher::exe_path()
{
    QFETCH(QString, cmd);
    QFETCH(QString, basedir);
    QFETCH(QString, expected);

    QCOMPARE(helpers::abs_launchcmd(cmd, basedir), expected);
}

void test_ProcessLauncher::exe_path_data()
{
    const QString app_path = QDir::currentPath();

    QTest::addColumn<QString>("cmd");
    QTest::addColumn<QString>("basedir");
    QTest::addColumn<QString>("expected");

    QTest::newRow("global") << "myapp" << "dummy" << "myapp";
#ifdef Q_OS_WIN
    QTest::newRow("relative A") << ".\\subdir\\app" << "c:\\some\\path" << "C:/some/path/subdir/app";
    QTest::newRow("relative B") << "./subdir/app" << "c:\\some\\path" << "C:/some/path/subdir/app";
    QTest::newRow("relative, no basedir A") << ".\\subdir\\app" << QString() << (app_path + "/subdir/app");
    QTest::newRow("relative, no basedir B") << "./subdir/app" << QString() << (app_path + "/subdir/app");
    QTest::newRow("absolute A ") << "c:\\subdir\\app" << "dummy" << "C:/subdir/app";
    QTest::newRow("absolute B") << "c:/subdir/app" << "dummy" << "C:/subdir/app";
#else
    QTest::newRow("relative") << "./subdir/app" << "/some/path" << "/some/path/subdir/app";
    QTest::newRow("relative, no basedir") << "./subdir/app" << QString() << (app_path + "/subdir/app");
    QTest::newRow("absolute") << "/subdir/app" << "dummy" << "/subdir/app";
#endif
}

void test_ProcessLauncher::workdir_path()
{
    QFETCH(QString, workdir);
    QFETCH(QString, basedir);
    QFETCH(QString, expected);

    QCOMPARE(helpers::abs_workdir(workdir, basedir, fallback_workdir()), expected);
}

void test_ProcessLauncher::workdir_path_data()
{
    const QString app_path = QDir::currentPath();

    QTest::addColumn<QString>("workdir");
    QTest::addColumn<QString>("basedir");
    QTest::addColumn<QString>("expected");

    QTest::newRow("null") << QString() << "dummy" << fallback_workdir();
#ifdef Q_OS_WIN
    QTest::newRow("relative A") << ".\\subdir" << "c:\\some\\path" << "C:/some/path/subdir";
    QTest::newRow("relative B") << "./subdir" << "c:\\some\\path" << "C:/some/path/subdir";
    QTest::newRow("relative, no basedir A") << ".\\subdir" << QString() << (app_path + "/subdir");
    QTest::newRow("relative, no basedir B") << "./subdir" << QString() << (app_path + "/subdir");
    QTest::newRow("absolute A") << "c:\\subdir" << "dummy" << "C:/subdir";
    QTest::newRow("absolute B") << "c:/subdir" << "dummy" << "C:/subdir";
#else
    QTest::newRow("relative") << "./subdir" << "/some/path" << "/some/path/subdir";
    QTest::newRow("relative, no basedir") << "./subdir" << QString() << (app_path + "/subdir");
    QTest::newRow("absolute") << "/subdir" << "dummy" << "/subdir";
#endif
}


QTEST_MAIN(test_ProcessLauncher)
#include "test_ProcessLauncher.moc"
