#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Button.hpp>
#include <Geode/modify/GJGarageLayer.hpp>

class $modify(StatsGarageLayer, GJGarageLayer) {
    static void onModify(auto& self) {
        if (!self.setHookPriority("GJGarageLayer::init", geode::Priority::FirstPost)) {
            geode::log::error("Failed to set hook priority for GJGarageLayer::init");
        }
    }
    struct Fields {
        cocos2d::CCMenu* m_statsMenu = nullptr;
        int m_prevActualChildren = 0;
        int m_currentPage = 0;
        int m_requestedPage = 0;
        std::vector<cocos2d::CCNode*> m_previousActualChildren = {};
        geode::Button* m_prevArrow;
        geode::Button* m_nextArrow;
    };
    bool init();

	void setupArrows();
    void pageChildren(float dt);
};
