#include <Geode/modify/GJGarageLayer.hpp>
#include <array>

using namespace geode::prelude;

class $modify(MyGJGarageLayer, GJGarageLayer) {
    bool init() {
        if (!GJGarageLayer::init())
            return false;

        static constexpr std::array<std::string_view, 14> garageIDsArray {
            "moons-icon",     "stars-icon",          "coins-icon",          "user-coins-icon",
            "orbs-icon",      "diamonds-icon",       "diamond-shards-icon", "moons-label",
            "stars-label",    "coins-label",         "user-coins-label",    "orbs-label",
            "diamonds-label", "diamond-shards-label"};

        for (auto garageID : garageIDsArray) {
            if (auto child = getChildByID(garageID)) {
                child->setVisible(false);
            }
        }

        return true;
    }
};
