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


#include "Filters.h"

#include "LocaleUtils.h"
#include "configfiles/CustomFilters.h"
#include "model/ListPropertyFn.h"
#include "utils/IndexShifter.h"

#include <QDebug>


namespace model {

Filter::Filter(QString name, QObject* parent)
    : QObject(parent)
    , m_name(std::move(name))
    , m_enabled(false)
{}


Filters::Filters(QObject* parent)
    : QObject(parent)
    , m_filters(CustomFilters::read())
    , m_filter_idx(m_filters.isEmpty() ? -1 : 0)
{
    for (Filter* const filter : qAsConst(m_filters)) {
        filter->setParent(this);
        connect(filter, &Filter::selectionChanged,
                this, &Filters::filtersChanged);
    }
}

Filter* Filters::current() const
{
    if (m_filter_idx < 0)
        return nullptr;

    Q_ASSERT(m_filter_idx < m_filters.length());
    return m_filters.at(m_filter_idx);
}

void Filters::setIndex(int idx)
{
    if (idx == m_filter_idx)
        return;

    const bool valid_idx = (idx == -1) || (0 <= idx && idx < m_filters.count());
    if (!valid_idx) {
        qWarning().noquote() << tr_log("Invalid filter index #%1").arg(idx);
        return;
    }

    m_filter_idx = idx;
    emit currentChanged();
}

void Filters::shiftIndex(IndexShiftDirection dir)
{
    if (m_filters.isEmpty())
        return;

    const int target_idx = shifterFn(dir)(m_filter_idx, m_filters.count());
    setIndex(target_idx);
}

void Filters::incrementIndex() {
    shiftIndex(IndexShiftDirection::INCREMENT);
}

void Filters::decrementIndex() {
    shiftIndex(IndexShiftDirection::DECREMENT);
}

void Filters::incrementIndexNoWrap() {
    shiftIndex(IndexShiftDirection::INCREMENT_NOWRAP);
}

void Filters::decrementIndexNoWrap() {
    shiftIndex(IndexShiftDirection::DECREMENT_NOWRAP);
}

QQmlListProperty<Filter> Filters::elementsProp()
{
    static constexpr auto count = &listproperty_count<Filter>;
    static constexpr auto at = &listproperty_at<Filter>;

    return {this, &m_filters, count, at};
}

} // namespace model
