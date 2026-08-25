#pragma once

#include <Geode/Result.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/menu_nodes/CCMenuItem.h>
#include <Geode/loader/Dispatch.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/ui/Button.hpp>
#include <Geode/utils/ZStringView.hpp>

#define MY_MOD_ID "capeling.garage-stats-menu"

constexpr float defaultNodeScale = 0.5f;

namespace stats_api {
using NodeProvider = geode::Function<cocos2d::CCNode*()>;
using ButtonProvider = geode::Function<geode::Button*()>;

inline bool isLoaded() {
    return geode::Loader::get()->getLoadedMod(MY_MOD_ID) != nullptr;
}

inline void registerStatItem(geode::ZStringView itemID,
                             NodeProvider provider,
                             int displayedNumber,
                             float nodeScale = defaultNodeScale)
    GEODE_EVENT_EXPORT_NORES(&registerStatItem, (itemID, std::move(provider), displayedNumber, nodeScale));

inline void registerStatItemButton(geode::ZStringView itemID,
                                   ButtonProvider provider,
                                   int displayedNumber,
                                   float nodeScale = defaultNodeScale)
    GEODE_EVENT_EXPORT_NORES(&registerStatItemButton, (itemID, std::move(provider), displayedNumber, nodeScale));

inline void updateDisplayNode(geode::ZStringView itemID, NodeProvider provider, float nodeScale = defaultNodeScale)
    GEODE_EVENT_EXPORT_NORES(&updateDisplayNode, (itemID, std::move(provider), nodeScale));

inline void updateDisplayButton(geode::ZStringView itemID, ButtonProvider provider, float nodeScale = defaultNodeScale)
    GEODE_EVENT_EXPORT_NORES(&updateDisplayButton, (itemID, std::move(provider), nodeScale));

inline void unregisterStatItem(geode::ZStringView itemID) GEODE_EVENT_EXPORT_NORES(&unregisterStatItem, (itemID));

inline geode::Result<int> getDisplayedNumber(geode::ZStringView itemID)
    GEODE_EVENT_EXPORT(&getDisplayedNumber, (itemID));

inline void setDisplayedNumber(geode::ZStringView itemID, int displayedNumber)
    GEODE_EVENT_EXPORT_NORES(&setDisplayedNumber, (itemID, displayedNumber));

} // namespace stats_api
