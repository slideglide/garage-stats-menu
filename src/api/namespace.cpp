#define GEODE_DEFINE_EVENT_EXPORTS
#include <Geode/Result.hpp>
#include <Geode/utils/ZStringView.hpp>
#include "api.hpp"

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
