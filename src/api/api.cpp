#define GEODE_DEFINE_EVENT_EXPORTS
#include "api.hpp"
#include <algorithm>
#include <stats_api.hpp>

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
    StatChangeEvent(id).send();
}

void StatsManager::updateDisplayNode(geode::ZStringView id, stats_api::NodeProvider provider, float scale) {
    if (auto it = m_stats.find(id); it != m_stats.end()) {
        it->second.provider = std::move(provider);
        it->second.nodeScale = scale;
        StatChangeEvent(id).send();
    }
}

void StatsManager::unregisterStatItem(geode::ZStringView id) {
    if (m_stats.erase(id) > 0) {
        StatChangeEvent(id).send();
    }
}

geode::Result<int> StatsManager::getDisplayedNumber(geode::ZStringView id) {
    auto it = m_stats.find(id);

    if (it == m_stats.end()) {
        return geode::Err("Stat item '{}' not found", id);
    }

    return geode::Ok(it->second.displayedNumber);
}

void StatsManager::setDisplayedNumber(geode::ZStringView id, int number) {
    if (auto it = m_stats.find(id); it != m_stats.end()) {
        if (it->second.displayedNumber != number) {
            it->second.displayedNumber = number;
            StatChangeEvent(id).send();
        }
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

namespace stats_api {
void registerStatItem(geode::ZStringView itemID, NodeProvider provider, int number, float scale) {
    StatsManager::get()->registerStatItem(itemID, std::move(provider), number, scale);
}

void registerStatItemButton(geode::ZStringView itemID, ButtonProvider provider, int number, float scale) {
    StatsManager::get()->registerStatItem(
        itemID, [bp = std::move(provider)]() mutable -> cocos2d::CCNode* { return bp ? bp() : nullptr; }, number,
        scale);
}

void updateDisplayNode(geode::ZStringView itemID, NodeProvider provider, float scale) {
    StatsManager::get()->updateDisplayNode(itemID, std::move(provider), scale);
}

void updateDisplayButton(geode::ZStringView itemID, ButtonProvider provider, float scale) {
    StatsManager::get()->updateDisplayNode(
        itemID, [bp = std::move(provider)]() mutable -> cocos2d::CCNode* { return bp ? bp() : nullptr; }, scale);
}

void unregisterStatItem(geode::ZStringView itemID) { StatsManager::get()->unregisterStatItem(itemID); }

geode::Result<int> getDisplayedNumber(geode::ZStringView itemID) {
    return StatsManager::get()->getDisplayedNumber(itemID);
}

void setDisplayedNumber(geode::ZStringView itemID, int number) {
    StatsManager::get()->setDisplayedNumber(itemID, number);
}
}  // namespace stats_api
