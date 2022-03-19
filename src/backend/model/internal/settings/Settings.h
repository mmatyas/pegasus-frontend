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


#pragma once

#include "AppSettings.h"
#include "KeyEditor.h"
#include "Locales.h"
#include "Themes.h"
#include "Providers.h"
#include "utils/QmlHelpers.h"

#include <QObject>


namespace model {

/// Provides a settings interface for the frontend layer
class Settings : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool fullscreen
               READ fullscreen WRITE setFullscreen
               NOTIFY fullscreenChanged)
    Q_PROPERTY(bool mouseSupport
               READ mouseSupport WRITE setMouseSupport
               NOTIFY mouseSupportChanged)
    Q_PROPERTY(bool verifyFiles
               READ verifyFiles WRITE setVerifyFiles
               NOTIFY verifyFilesChanged)
    Q_PROPERTY(QStringList gameDirs READ gameDirs NOTIFY gameDirsChanged)
    Q_PROPERTY(QStringList androidGrantedDirs READ androidGrantedDirs NOTIFY androidDirsChanged)

    QML_CONST_PROPERTY(model::KeyEditor, keyEditor)
    QML_CONST_PROPERTY(model::Locales, locales)
    QML_CONST_PROPERTY(model::Themes, themes)
    QML_CONST_PROPERTY(model::Providers, providers)

public:
    explicit Settings(QObject* parent = nullptr);
    void postInit();

    bool fullscreen() const { return AppSettings::general.fullscreen; }
    void setFullscreen(bool);

    bool mouseSupport() const { return AppSettings::general.mouse_support; }
    void setMouseSupport(bool);

    bool verifyFiles() const { return AppSettings::general.verify_files; }
    void setVerifyFiles(bool);

    QStringList gameDirs() const;
    Q_INVOKABLE void addGameDir(const QString&);
    Q_INVOKABLE void removeGameDirs(const QVariantList&);

    QStringList androidGrantedDirs() const;
    Q_INVOKABLE void requestAndroidDir();

    Q_INVOKABLE void reloadProviders();

signals:
    void fullscreenChanged();
    void mouseSupportChanged();
    void verifyFilesChanged();
    void gameDirsChanged();
    void androidDirsChanged();
    void providerReloadingRequested();
};

} // namespace model
