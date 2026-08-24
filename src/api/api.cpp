#include "api.hpp"
#include <stats_api.hpp>
#include <algorithm>

StatsManager* StatsManager::get() {
    static StatsManager instance;
    return &instance;
}

void StatsManager::registerStatItem(geode::ZStringView id, stats_api::NodeProvider provider, int number, float scale) {
    m_stats.insert_or_assign(id, StatItem{
        .provider = std::move(provider),
        .displayedNumber = number,
        .nodeScale = scale,
    });
}

void StatsManager::updateDisplayNode(geode::ZStringView id, stats_api::NodeProvider provider, float scale) {
    if (auto it = m_stats.find(id); it != m_stats.end()) {
        it->second.provider = std::move(provider);
        it->second.nodeScale = scale;
    }
}

void StatsManager::unregisterStatItem(geode::ZStringView id) { m_stats.erase(id); }

geode::Result<int> StatsManager::getDisplayedNumber(geode::ZStringView id) {
    auto it = m_stats.find(id);

    if (it == m_stats.end()) {
        return geode::Err("Stat item '{}' not found", id);
    }

    return geode::Ok(it->second.displayedNumber);
}

void StatsManager::setDisplayedNumber(geode::ZStringView id, int number) {
    if (auto it = m_stats.find(id); it != m_stats.end()) {
        it->second.displayedNumber = number;
    }
}

void StatsManager::forEachStat(StatCallback callback) {
    std::vector<std::string> sortedKeys;
    sortedKeys.reserve(m_stats.size());

    for (auto const& [key, _] : m_stats) {
        sortedKeys.push_back(key);
    }

    std::sort(sortedKeys.begin(), sortedKeys.end());

    for (auto const& key : sortedKeys) {
        if (auto it = m_stats.find(key); it != m_stats.end()) {
            callback(key, it->second);
        }
    }
}