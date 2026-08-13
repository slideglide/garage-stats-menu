#include <Geode/modify/GJGarageLayer.hpp>
#include <vector>
#include <string_view>

class $modify(MyGJGarageLayer, GJGarageLayer) {
    bool init() {
        if (!GJGarageLayer::init()) 
            return false;

        static std::vector<std::string_view> garageIDsVector {
            "moons-icon",
            "stars-icon",
            "coins-icon",
            "user-coins-icon",
            "orbs-icon",
            "diamonds-icon",
            "diamond-shards-icon",
            "moons-label",
            "stars-label",
            "user-coins-label",
            "orbs-label",
            "diamonds-label",
            "diamond-shards-label"
        };

        for (const auto& garageIDs : garageIDsVector) {
            auto child = getChildByID(garageIDs);
            if (child) {
                child->setVisible(false);
            }
        }

        return true;
    }
};