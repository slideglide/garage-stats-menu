#define GEODE_DEFINE_EVENT_EXPORTS
#include "api.hpp"
#include <StatsDisplayAPI.h>
#include <algorithm>

StatsManager* StatsManager::get() {
    static StatsManager instance;
    return &instance;
}

void StatsManager::registerStatItem(geode::ZStringView id, cocos2d::CCNode* node, int number, float scale) {
    m_stats.insert_or_assign(id, StatItem{
        .displayNode = node,
        .displayedNumber = number,
        .nodeScale = scale,
    });
}

void StatsManager::updateDisplayNode(geode::ZStringView id, cocos2d::CCNode* node, float scale) {
    if (auto it = m_stats.find(id); it != m_stats.end()) {
        it->second.displayNode = node;
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

std::vector<std::pair<std::string, StatItem>> StatsManager::getManagedStats() const {
    std::vector<std::pair<std::string, StatItem>> result;
    result.reserve(m_stats.size());

    for (auto const& [key, val] : m_stats) {
        result.emplace_back(key, val);
    }

    std::sort(result.begin(), result.end(), [](auto const& a, auto const& b) {
        return a.first < b.first;
    });

    return result;
}

namespace stats_api {
void registerStatItem(geode::ZStringView statItemId, cocos2d::CCNode* node, int number, float scale) {
    StatsManager::get()->registerStatItem(statItemId, node, number, scale);
}

void updateDisplayNode(geode::ZStringView statItemId, cocos2d::CCNode* node, float scale) {
    StatsManager::get()->updateDisplayNode(statItemId, node, scale);
}

void unregisterStatItem(geode::ZStringView statItemId) { StatsManager::get()->unregisterStatItem(statItemId); }

geode::Result<int> getDisplayedNumber(geode::ZStringView statItemId) {
    return StatsManager::get()->getDisplayedNumber(statItemId);
}

void setDisplayedNumber(geode::ZStringView statItemId, int number) {
    StatsManager::get()->setDisplayedNumber(statItemId, number);
}
}  // namespace stats_api
