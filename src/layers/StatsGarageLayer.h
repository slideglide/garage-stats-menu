#pragma once

#ifndef STATSGARAGELAYER_H
#define STATSGARAGELAYER_H

#include <Geode/Geode.hpp>
#include <Geode/modify/GJGarageLayer.hpp>
#include "../Macros.h"

class $modify(StatsGarageLayer, GJGarageLayer) {
	struct Fields {
		cocos2d::CCMenu* m_statsMenu = nullptr;
		int m_prevActualChildren = 0;
		int m_currentPage = 0;
		int m_requestedPage = 0;
		std::vector<cocos2d::CCNode*> m_previousActualChildren = {};
		CCMenuItemSpriteExtra* m_prevArrow;
		CCMenuItemSpriteExtra* m_nextArrow;
	};
	$override
	bool init();
	
	void switchPage(CCObject*);
	void pageChildren(float);


	EARLY_MODIFY(GJGarageLayer::init);
};
#endif