#pragma once

#include <StatsDisplayAPI.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/Result.hpp>
#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/ZStringView.hpp>
#include <Geode/utils/cocos.hpp>
#include <string>
#include <utility>
#include <vector>

struct StatItem {
    geode::WeakRef<cocos2d::CCNode> displayNode;
    int displayedNumber;
    float nodeScale;
};

class StatsManager {
private:
    geode::utils::StringMap<StatItem> m_stats;

public:
    static StatsManager* get();

    void registerStatItem(geode::ZStringView statItemId, cocos2d::CCNode* node, int number, float scale);
    void updateDisplayNode(geode::ZStringView statItemId, cocos2d::CCNode* node, float scale);
    void unregisterStatItem(geode::ZStringView statItemId);

    geode::Result<int> getDisplayedNumber(geode::ZStringView statItemId);
    void setDisplayedNumber(geode::ZStringView statItemId, int number);

    std::vector<std::pair<std::string, StatItem>> getManagedStats() const;

    auto const& getStats() const { return m_stats; }
};
