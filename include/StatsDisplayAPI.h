#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Label.hpp>

namespace StatsDisplayAPI {
    inline cocos2d::CCNode* getNewItem(std::string ID,
                                       cocos2d::CCNode* displayNode,
                                       int displayedNum,
                                       float nodeScale = 0.5f) {
        auto ret = cocos2d::CCMenu::create();

        displayNode->setID(ID + "-icon");
        displayNode->setScale(nodeScale);
        displayNode->removeFromParentAndCleanup(false);
        displayNode->setPosition({0, 0});
        ret->addChild(displayNode);

        auto label = geode::Label::create(fmt::to_string(displayedNum), "bigFont.fnt");
        label->setID(ID + "-label");
        label->setScale(0.34f);
        label->setAnchorPoint({1, 0.5});
        label->removeFromParentAndCleanup(false);
        label->setPosition({-12, 0.5});
        ret->addChild(label);

        ret->setID(ID + "-container");
        ret->setContentSize({0, 0});
        return ret;
    }
}; // namespace StatsDisplayAPI