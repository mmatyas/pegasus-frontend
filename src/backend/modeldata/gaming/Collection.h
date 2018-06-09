#pragma once

#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <vector>


namespace modeldata {

struct Game;
using GamePtr = QSharedPointer<modeldata::Game>;

struct Collection {
    explicit Collection(QString name);

    const QString& name() const { return m_name; }
    const QString& shortName() const { return m_short_name; }
    const QString& launchCmd() const { return m_launch_cmd; }
    void setShortName(const QString&);
    void setLaunchCmd(QString);

    const std::vector<GamePtr>& games() const { return m_games; }
    std::vector<GamePtr>& gamesMut() { return m_games; }
    void sortGames();

    QStringList source_dirs; // TODO: remove


    Collection(const Collection&) = delete;
    Collection& operator=(const Collection&) = delete;
    Collection(Collection&&) = default;
    Collection& operator=(Collection&&) = default;

private:
    QString m_name;
    QString m_short_name;
    QString m_launch_cmd;

    std::vector<GamePtr> m_games;
};

} // namespace modeldata
