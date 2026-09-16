#pragma once

#include <Geode/Result.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/utils/function.hpp>
#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/ZStringView.hpp>
#include <Geode/utils/cocos.hpp>
#include <stats_api.hpp>

class StatChangeEvent : public geode::GlobalEvent<StatChangeEvent, bool(), std::string_view> {
public:
    using GlobalEvent::GlobalEvent;
};

struct StatItem {
    stats_api::NodeProvider provider;
    int displayedNumber;
    float nodeScale;
};

class StatsManager {
private:
    geode::utils::StringMap<StatItem> m_stats;

public:
    using StatCallback = geode::Function<void(std::string_view key, StatItem& item)>;

    static StatsManager* get();

    void registerStatItem(geode::ZStringView itemID, stats_api::NodeProvider provider, int number, float scale);
    bool isStatItemRegistered(geode::ZStringView itemID);
    void updateDisplayNode(geode::ZStringView itemID, stats_api::NodeProvider provider, float scale);
    void unregisterStatItem(geode::ZStringView itemID);

    geode::Result<int> getDisplayedNumber(geode::ZStringView itemID);
    void setDisplayedNumber(geode::ZStringView itemID, int number);

    void forEachStat(StatCallback callback);

    auto const& getStats() const { return m_stats; }
};
