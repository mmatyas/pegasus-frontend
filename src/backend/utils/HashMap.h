#pragma once

#include <QHash>
#include <QString>
#include <unordered_map>


// std::unordered_map is too verbose
template <typename Key, typename Val, typename Hash = std::hash<Key>>
using HashMap = std::unordered_map<Key, Val, Hash>;

// hash for strings
namespace std {
    template<> struct hash<QString> {
        std::size_t operator()(const QString& s) const {
            return qHash(s);
        }
    };
    template<> struct hash<QLatin1String> {
        std::size_t operator()(const QLatin1String& s) const {
            return qHash(s);
        }
    };
}

// hash for enum classes
struct EnumHash {
    template <typename T>
    std::size_t operator()(T key) const {
        return static_cast<std::size_t>(key);
    }
};
