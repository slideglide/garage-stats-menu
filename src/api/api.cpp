#define GEODE_DEFINE_EVENT_EXPORTS
#include "api.hpp"
#include <algorithm>
#include <utility>
#include <stats_api.hpp>

using namespace geode::prelude;

StatsManager* StatsManager::get() {
    static StatsManager instance;
    return &instance;
}

void StatsManager::registerStatItem(
    ZStringView itemID, 
    stats_api::NodeProvider provider, 
    int number, 
    float scale
) {
    if (auto it = m_stats.find(itemID); it != m_stats.end()) {
        it->second.provider = std::move(provider);
        it->second.displayedNumber = number;
        it->second.nodeScale = scale;
    } else {
        m_stats.emplace(std::string(itemID), StatItem{
            .provider = std::move(provider),
            .displayedNumber = number,
            .nodeScale = scale
        });
    }

    StatChangeEvent(itemID).send();
}

bool StatsManager::isStatItemRegistered(ZStringView itemID) {
    return m_stats.contains(itemID);
}

void StatsManager::updateDisplayNode(
    ZStringView itemID,
    stats_api::NodeProvider provider, 
    std::optional<float> scale
) {
    if (auto it = m_stats.find(itemID); it != m_stats.end()) {
        it->second.provider = std::move(provider);
        if (scale) it->second.nodeScale = *scale;
        StatChangeEvent(itemID).send();
        return;
    }

    log::error("Failed to update display node: stat item '{}' not found", itemID);
}

void StatsManager::unregisterStatItem(ZStringView itemID) {
    if (m_stats.erase(itemID) > 0) {
        StatChangeEvent(itemID).send();
    }
}

Result<int> StatsManager::getDisplayedNumber(ZStringView itemID) {
    if (auto it = m_stats.find(itemID); it != m_stats.end()) {
        return Ok(it->second.displayedNumber);
    }

    return Err("Stat item '{}' not found", itemID);
}

void StatsManager::setDisplayedNumber(ZStringView itemID, int number) {
    if (auto it = m_stats.find(itemID); it != m_stats.end()) {
        if (it->second.displayedNumber != number) {
            it->second.displayedNumber = number;
            StatChangeEvent(itemID).send();
        }
        return;
    }

    log::error("Failed to set displayed number: stat item '{}' not found", itemID);
}

void StatsManager::forEachStat(StatCallback callback) {
    std::vector<std::string> keys;
    keys.reserve(m_stats.size());

    for (auto const& [key, _] : m_stats) {
        keys.push_back(key);
    }

    std::sort(keys.begin(), keys.end());

    for (auto const& key : keys) {
        auto it = m_stats.find(key);
        if (it == m_stats.end()) continue;

        StatItem item{
            .provider = std::exchange(it->second.provider, nullptr),
            .displayedNumber = it->second.displayedNumber,
            .nodeScale = it->second.nodeScale
        };
        callback(key, item);

        if (it = m_stats.find(key); it != m_stats.end() && !it->second.provider) {
            it->second.provider = std::move(item.provider);
        }
    }
}

namespace stats_api {
void registerStatItem(
    ZStringView itemID, 
    NodeProvider provider, 
    int number, 
    float scale
) {
    StatsManager::get()->registerStatItem(itemID, std::move(provider), number, scale);
}

void registerStatItemButton(
    ZStringView itemID, 
    ButtonProvider provider, 
    int number, 
    float scale
) {
    StatsManager::get()->registerStatItem(itemID, NodeProvider(std::move(provider)), number, scale);
}

bool isStatItemRegistered(geode::ZStringView itemID) {
    return StatsManager::get()->isStatItemRegistered(itemID);
}

void updateDisplayNode(
    ZStringView itemID, 
    NodeProvider provider, 
    std::optional<float> scale
) {
    StatsManager::get()->updateDisplayNode(itemID, std::move(provider), scale);
}

void updateDisplayButton(
    ZStringView itemID, 
    ButtonProvider provider, 
    std::optional<float> scale
) {
    StatsManager::get()->updateDisplayNode(itemID, NodeProvider(std::move(provider)), scale);
}

void unregisterStatItem(ZStringView itemID) { 
    StatsManager::get()->unregisterStatItem(itemID); 
}

Result<int> getDisplayedNumber(ZStringView itemID) {
    return StatsManager::get()->getDisplayedNumber(itemID);
}

void setDisplayedNumber(ZStringView itemID, int number) {
    StatsManager::get()->setDisplayedNumber(itemID, number);
}
}  // namespace stats_api
