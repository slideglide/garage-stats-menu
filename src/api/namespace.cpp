#define GEODE_DEFINE_EVENT_EXPORTS
#include <Geode/Result.hpp>
#include <Geode/utils/ZStringView.hpp>
#include <stats_api.hpp>
#include "api.hpp"

namespace stats_api {
void registerStatItem(geode::ZStringView statItemId, NodeProvider provider, int number, float scale) {
    StatsManager::get()->registerStatItem(statItemId, std::move(provider), number, scale);
}

void registerStatItemButton(geode::ZStringView statItemId, ButtonProvider buttonProvider, int number, float scale) {
    StatsManager::get()->registerStatItem(statItemId, [bp = std::move(buttonProvider)]() mutable -> cocos2d::CCNode* {
        return bp ? bp() : nullptr;
    }, number, scale);
}

void updateDisplayNode(geode::ZStringView statItemId, NodeProvider provider, float scale) {
    StatsManager::get()->updateDisplayNode(statItemId, std::move(provider), scale);
}

void unregisterStatItem(geode::ZStringView statItemId) { StatsManager::get()->unregisterStatItem(statItemId); }

geode::Result<int> getDisplayedNumber(geode::ZStringView statItemId) {
    return StatsManager::get()->getDisplayedNumber(statItemId);
}

void setDisplayedNumber(geode::ZStringView statItemId, int number) {
    StatsManager::get()->setDisplayedNumber(statItemId, number);
}
}  // namespace stats_api