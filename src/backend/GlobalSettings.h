#pragma once

#include <QString>
#include <functional>


class GlobalSettings {
public:
    GlobalSettings();

    static void parse_gamedirs(const std::function<void(const QString&)>&);
};
