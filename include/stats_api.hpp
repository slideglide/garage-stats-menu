#pragma once

#include <Geode/Result.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/loader/Dispatch.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/ui/Button.hpp>
#include <Geode/utils/ZStringView.hpp>
#include <optional>

#define MY_MOD_ID "capeling.garage-stats-menu"

namespace stats_api {

using NodeProvider = geode::Function<cocos2d::CCNode*()>;
using ButtonProvider = geode::Function<geode::Button*()>;

GEODE_EVENT_EXPORT(
    void, registerStatItem,
    (geode::ZStringView itemID, NodeProvider provider, int displayedNumber, float nodeScale = 0.5f),
    (itemID, std::move(provider), displayedNumber, nodeScale)
);

GEODE_EVENT_EXPORT(
    void, registerStatItemButton,
    (geode::ZStringView itemID, ButtonProvider provider, int displayedNumber, float nodeScale = 0.5f),
    (itemID, std::move(provider), displayedNumber, nodeScale)
);

GEODE_EVENT_EXPORT(
    bool, isStatItemRegistered,
    (geode::ZStringView itemID),
    (itemID)
);

GEODE_EVENT_EXPORT(
    void, updateDisplayNode,
    (geode::ZStringView itemID, NodeProvider provider, std::optional<float> nodeScale = std::nullopt),
    (itemID, std::move(provider), nodeScale)
);

GEODE_EVENT_EXPORT(
    void, updateDisplayButton,
    (geode::ZStringView itemID, ButtonProvider provider, std::optional<float> nodeScale = std::nullopt),
    (itemID, std::move(provider), nodeScale)
);

GEODE_EVENT_EXPORT(
    void, unregisterStatItem,
    (geode::ZStringView itemID),
    (itemID)
);

GEODE_EVENT_EXPORT(
    geode::Result<int>, getDisplayedNumber,
    (geode::ZStringView itemID),
    (itemID)
);

GEODE_EVENT_EXPORT(
    void, setDisplayedNumber,
    (geode::ZStringView itemID, int displayedNumber),
    (itemID, displayedNumber)
);

} // namespace stats_api
