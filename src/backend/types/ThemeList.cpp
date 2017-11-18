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


#include "ThemeList.h"

#include "ListPropertyFn.h"
#include "model_providers/AppFiles.h"

#include <QDebug>
#include <QSettings>


namespace {

const QLatin1String SETTINGSKEY_THEME("theme");

} // namespace


namespace Types {

ThemeList::ThemeList(QObject* parent)
    : QObject(parent)
    , m_themes(model_providers::AppFiles::findAvailableThemes())
    , m_theme_idx(-1)
{
    for (Theme* theme : qAsConst(m_themes)) {
        theme->setParent(this);
        qInfo().noquote() << tr("Found theme '%1' (`%2`)").arg(theme->name(), theme->dir());
    }

    selectPreferredTheme();
    printChangeMsg();
}

Theme* ThemeList::current() const
{
    Q_ASSERT(0 <= index() && index() < m_themes.length());
    return m_themes.at(index());
}

int ThemeList::indexOfTheme(const QString& dir_path) const
{
    for (int idx = 0; idx < m_themes.count(); idx++) {
        if (m_themes.at(idx)->dir() == dir_path)
            return idx;
    }

    return -1;
}

void ThemeList::selectPreferredTheme()
{
    // this method should be called after all themes have been found
    Q_ASSERT(!m_themes.isEmpty());


    // A. Try to use the saved config value
    const QString requested_theme = QSettings().value(SETTINGSKEY_THEME).toString();
    if (!requested_theme.isEmpty())
        m_theme_idx = indexOfTheme(requested_theme);

    // B. Fall back to the built-in theme
    //    Either the config value is invalid, or has missing files,
    //    thus not present in `m_themes`.
    if (m_theme_idx < 0)
        m_theme_idx = indexOfTheme(":/themes/pegasus-grid/");


    Q_ASSERT(m_theme_idx >= 0 && m_theme_idx < m_themes.length());
}

void ThemeList::setIndex(int idx)
{
    // verify
    if (idx == m_theme_idx)
        return;

    const bool valid_idx = (0 <= idx && idx < m_themes.length());
    if (!valid_idx) {
        qWarning() << tr("Invalid theme index #%1").arg(idx);
        return;
    }

    // set
    m_theme_idx = idx;
    printChangeMsg();
    emit themeChanged();

    // remember
    QSettings().setValue(SETTINGSKEY_THEME, m_themes.at(idx)->dir());
}

void ThemeList::printChangeMsg() const
{
    qInfo().noquote() << QObject::tr("Theme set to '%1' (%2)")
                         .arg(current()->name(), current()->dir());
}

QQmlListProperty<Theme> ThemeList::getListProp()
{
    static const auto count = &listproperty_count<Theme>;
    static const auto at = &listproperty_at<Theme>;

    return {this, &m_themes, count, at};
}

} // namespace Types
