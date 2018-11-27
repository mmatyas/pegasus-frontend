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


import QtQuick 2.0


Error {
    title: qsTr("Theme loading failed :(") + api.tr
    details: qsTr("Pegasus tried to load the selected theme (%1), but failed."
                + " This may happen when you try to load an outdated theme, or when"
                + " there's a bug in its code.")
            .arg(api.internal.settings.themes.current.name)
            + api.tr
    instruction: qsTr("For now, please select a different theme from the main menu.") + api.tr
    logInfo: qsTr("You might find more details in the log file:<pre>%1</pre>")
        .arg(api.internal.meta.logFilePath)
        + api.tr
}
