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


#include "FrontendLayer.h"

#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QQmlContext>
#include <QQmlNetworkAccessManagerFactory>
#include <QStandardPaths>


namespace {

class DiskCachedNAMFactory : public QQmlNetworkAccessManagerFactory {
public:
    QNetworkAccessManager* create(QObject* parent) override;
};

QNetworkAccessManager* DiskCachedNAMFactory::create(QObject* parent)
{
    QNetworkAccessManager* nam = new QNetworkAccessManager(parent);
    QNetworkDiskCache* cache = new QNetworkDiskCache(nam);

    QString cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    cache_path += QLatin1String("/netcache");
    cache->setCacheDirectory(cache_path);

    nam->setCache(cache);
    return nam;
}

} // namespace


FrontendLayer::FrontendLayer(QObject* parent)
    : QObject(parent)
{
}

void FrontendLayer::rebuild(QObject* api)
{
    Q_ASSERT(api);
    Q_ASSERT(!engine);

    engine = new QQmlApplicationEngine();
    engine->addImportPath("lib/qml");
    engine->addImportPath("qml");
    engine->setNetworkAccessManagerFactory(new DiskCachedNAMFactory);
    engine->rootContext()->setContextProperty(QStringLiteral("pegasus"), api);
    engine->load(QUrl(QStringLiteral("qrc:/frontend/main.qml")));

    emit rebuildComplete();
}

void FrontendLayer::teardown()
{
    Q_ASSERT(engine);

    // signal forwarding
    connect(engine.data(), &QQmlApplicationEngine::destroyed,
            this, &FrontendLayer::teardownComplete);

    engine->deleteLater();
}
