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

#include "model/internal/System.h"

Q_DECLARE_METATYPE(AppCloseType)


class test_System : public QObject {
    Q_OBJECT

private slots:
    void appClose();
    void appClose_data();
};

void test_System::appClose()
{
    model::System system;
    QSignalSpy spy(&system, &model::System::appCloseRequested);
    QVERIFY(spy.isValid());

    QFETCH(AppCloseType, close_type);
    switch (close_type) {
        case AppCloseType::QUIT: system.quit(); break;
        case AppCloseType::REBOOT: system.reboot(); break;
        case AppCloseType::SHUTDOWN: system.shutdown(); break;
    }

    QCOMPARE(spy.count(), 1);
    QVERIFY(!spy.isEmpty() && !spy.constFirst().isEmpty()
            && spy.takeFirst().first() == close_type);
}

void test_System::appClose_data()
{
    QTest::addColumn<AppCloseType>("close_type");

    QTest::newRow("quit") << AppCloseType::QUIT;
    QTest::newRow("shutdown") << AppCloseType::SHUTDOWN;
    QTest::newRow("reboot") << AppCloseType::REBOOT;
}


QTEST_MAIN(test_System)
#include "test_System.moc"
