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

#include <QObject>
#include <QVector>
#include <QQmlListProperty>
#include <QRegularExpression>

enum class IndexShiftDirection : unsigned char;


namespace model {

enum class FilterRuleType : unsigned char {
    IS_TRUE,
    IS_FALSE,
    EMPTY,
    NOT_EMPTY,
    CONTAINS,
    NOT_CONTAINS,
    EQUALS,
    NOT_EQUALS,
};

struct FilterRule {
    QLatin1String game_property;
    FilterRuleType type;
    QRegularExpression regex;
};


class Filter : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(bool enabled MEMBER m_enabled NOTIFY selectionCanged)

public:
    explicit Filter(QString name, QObject* parent = nullptr);

    bool enabled() const { return m_enabled; }

    const QVector<FilterRule>& rules() const { return m_rules; }
    QVector<FilterRule>& rulesMut() { return m_rules; }

signals:
    void selectionCanged();

private:
    const QString m_name;
    bool m_enabled;
    QVector<FilterRule> m_rules;
};


/// Stores parameters to filter the list of games
class Filters : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString gameTitle MEMBER m_game_title NOTIFY filtersChanged)

    Q_PROPERTY(model::Filter* current
               READ current
               NOTIFY currentChanged)
    Q_PROPERTY(int index
               READ index
               WRITE setIndex
               NOTIFY currentChanged)
    Q_PROPERTY(int count
               READ count
               NOTIFY modelChanged)
    Q_PROPERTY(QQmlListProperty<model::Filter> model
               READ elementsProp
               NOTIFY modelChanged)

public:
    explicit Filters(QObject* parent = nullptr);

    QString m_game_title;

    Filter* current() const;
    int count() const { return m_filters.count(); }
    int index() const { return m_filter_idx; }
    void setIndex(int);
    Q_INVOKABLE void incrementIndex();
    Q_INVOKABLE void decrementIndex();
    Q_INVOKABLE void incrementIndexNoWrap();
    Q_INVOKABLE void decrementIndexNoWrap();

    QQmlListProperty<Filter> elementsProp();
    const QVector<Filter*>& elements() const { return m_filters; }
    QVector<Filter*>& elementsMut() { return m_filters; }

    void lock();

signals:
    void filtersChanged();
    void currentChanged();
    void modelChanged();

private:
    QVector<Filter*> m_filters;
    int m_filter_idx;

    void shiftIndex(IndexShiftDirection);
};

} // namespace model
