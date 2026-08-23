#pragma once

#include <Geode/Result.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/loader/Dispatch.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/ZStringView.hpp>

#define MY_MOD_ID "capeling.garage-stats-menu"

constexpr float defaultNodeScale = 0.5f;

namespace stats_api {

inline void registerStatItem(geode::ZStringView statItemId, cocos2d::CCNode* displayNode, int displayedNumber,
                             float nodeScale = defaultNodeScale)
    GEODE_EVENT_EXPORT_NORES(&registerStatItem, (statItemId, displayNode, displayedNumber, nodeScale));

inline void updateDisplayNode(geode::ZStringView statItemId, cocos2d::CCNode* displayNode,
                              float nodeScale = defaultNodeScale)
    GEODE_EVENT_EXPORT_NORES(&updateDisplayNode, (statItemId, displayNode, nodeScale));

inline void unregisterStatItem(geode::ZStringView statItemId)
    GEODE_EVENT_EXPORT_NORES(&unregisterStatItem, (statItemId));

inline geode::Result<int> getDisplayedNumber(geode::ZStringView statItemId)
    GEODE_EVENT_EXPORT(&getDisplayedNumber, (statItemId));

inline void setDisplayedNumber(geode::ZStringView statItemId, int displayedNumber)
    GEODE_EVENT_EXPORT_NORES(&setDisplayedNumber, (statItemId, displayedNumber));

} // namespace stats_api

namespace StatsDisplayAPI {
[[deprecated(
    "Use `stats_api::registerStatItem` in an $execute block instead, this API will be removed in the next GD update.")]]
inline cocos2d::CCNode* getNewItem(const std::string& ID, cocos2d::CCNode* displayNode, int displayedNum,
                                   float nodeScale = defaultNodeScale) {
    auto* ret = cocos2d::CCMenu::create();

    if (displayNode != nullptr) {
        displayNode->setID(ID + "-icon");
        displayNode->setScale(nodeScale);
        if (displayNode->getParent() != nullptr) {
            displayNode->removeFromParentAndCleanup(false);
        }
        displayNode->setPosition({0, 0});
        ret->addChild(displayNode);
    }

    geode::Label* label = geode::Label::create(fmt::to_string(displayedNum), "bigFont.fnt");
    label->setID(ID + "-label");
    label->setScale(0.34f);
    label->setAnchorPoint({1.0f, 0.5f});
    label->setPosition({-12.0f, 0.5f});
    ret->addChild(label);

    ret->setID(ID + "-container");
    ret->setContentSize({0, 0});

    stats_api::registerStatItem(ID, displayNode, displayedNum, nodeScale);

    return ret;
}

} // namespace StatsDisplayAPI
