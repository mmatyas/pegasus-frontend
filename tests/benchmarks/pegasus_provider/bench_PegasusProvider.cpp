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

#include "Log.h"
#include "providers/SearchContext.h"
#include "providers/pegasus_metadata/PegasusProvider.h"

#include <QString>


class bench_PegasusProvider : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        Log::init_qttest();
    }

    void find_in_empty_dir();
    void find_in_filled_dir();
};

void bench_PegasusProvider::find_in_empty_dir()
{
    providers::SearchContext sctx({QStringLiteral(":/empty")});
    providers::pegasus::PegasusProvider provider;

    QBENCHMARK {
        QTest::ignoreMessage(QtInfoMsg, "Pegasus Metafiles: No metadata files found");
        provider.run(sctx);
    }
}

void bench_PegasusProvider::find_in_filled_dir()
{
    providers::SearchContext sctx({QStringLiteral(":/filled")});
    providers::pegasus::PegasusProvider provider;

    const QString msg = QStringLiteral("Pegasus Metafiles: Found `%1`")
        .arg(QDir::toNativeSeparators(QStringLiteral(":/filled/metadata.pegasus.txt")));

    QBENCHMARK {
        QTest::ignoreMessage(QtInfoMsg, qUtf8Printable(msg));
        provider.run(sctx);
    }
}


QTEST_MAIN(bench_PegasusProvider)
#include "bench_PegasusProvider.moc"
