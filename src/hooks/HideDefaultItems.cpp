#include <Geode/modify/GJGarageLayer.hpp>

using namespace geode::prelude;

class $modify(HideDefaultItemsLayer, GJGarageLayer) {
    bool init() {
        if (!GJGarageLayer::init()) {
            return false;
        }

        constexpr auto garageIDs = std::to_array<std::string_view>({
            "moons-icon",       "stars-icon",          "coins-icon",     "user-coins-icon",     "orbs-icon",
            "diamonds-icon",    "diamond-shards-icon", "moons-label",    "stars-label",         "coins-label",
            "user-coins-label", "orbs-label",          "diamonds-label", "diamond-shards-label"});

        for (auto id : garageIDs) {
            if (auto child = getChildByID(id)) {
                child->setVisible(false);
            }
        }

        return true;
    }
};
