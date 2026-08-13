#include "StatsGarageLayer.h"
#include <StatsDisplayAPI.h>

using namespace geode::prelude;

const int ELEMENTS_PER_PAGE = 10;
const float TOP_MARGIN = 6.f;
float RIGHT_MARGIN = 18.f;
float ARROW_SCALE = 0.6f;

inline void addDefaultItem(
	CCNode* statsMenu,
	const char* setting,
	const char* id,
	const char* spriteFrameName,
	const char* statNum,
	float scale
) {
	if (Mod::get()->getSettingValue<bool>(setting)) {
		statsMenu->addChild(
			StatsDisplayAPI::getNewItem(
				id,
				CCSprite::createWithSpriteFrameName(spriteFrameName),
				GameStatsManager::sharedState()->getStat(statNum),
				scale
			)
		);
	}
}

bool StatsGarageLayer::init() {
	if (!GJGarageLayer::init())
		return false;

	auto fields = m_fields.self();
	auto winSize = CCDirector::get()->getWinSize();
	auto mod = Mod::get();

	fields->m_statsMenu = CCMenu::create();

	fields->m_statsMenu->setID("stats-menu"_spr);
	fields->m_statsMenu->setZOrder(2);
	fields->m_statsMenu->setLayout(
		ColumnLayout::create()
		->setAxisReverse(true)
		->setCrossAxisAlignment(AxisAlignment::End)
		->setAxisAlignment(AxisAlignment::End)
		->setGap(15)
	);
	
	this->addChild(fields->m_statsMenu);
	
	addDefaultItem(fields->m_statsMenu, "stars-stat", "stars", "GJ_starsIcon_001.png", "6", 0.54f);
	addDefaultItem(fields->m_statsMenu, "moons-stat", "moons", "GJ_moonsIcon_001.png", "28", 0.54f);
	addDefaultItem(fields->m_statsMenu, "gold-coins-stat", "coins", "GJ_coinsIcon_001.png", "8", 0.51f);
	addDefaultItem(fields->m_statsMenu, "user-coins-stat", "user-coins", "GJ_coinsIcon2_001.png", "12", 0.51f);
	addDefaultItem(fields->m_statsMenu, "orbs-stat", "orbs", "currencyOrbIcon_001.png", "14", 0.54f);
	addDefaultItem(fields->m_statsMenu, "diamonds-stat", "diamonds", "GJ_diamondsIcon_001.png", "13", 0.6f);
	addDefaultItem(fields->m_statsMenu, "diamond-shards-stat", "diamond-shards", "currencyDiamondIcon_001.png", "29", 0.54f);
	
	float bottomMargin = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png")->getContentHeight();
	float arrowSize = ARROW_SCALE * CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png")->getContentWidth();
	
	float maxHeight = winSize.height - TOP_MARGIN - bottomMargin;
	fields->m_statsMenu->setContentSize({0, maxHeight});
	fields->m_statsMenu->setAnchorPoint(CCPoint{0.5f, 1.f});
	fields->m_statsMenu->setPosition(CCPoint{winSize.width - RIGHT_MARGIN, winSize.height - TOP_MARGIN});
	
	CCSprite* prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	CCSprite* nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	nextSprite->setFlipX(true);
	prevSprite->setRotation(90);
	nextSprite->setRotation(90);
	prevSprite->setScale(ARROW_SCALE);
	nextSprite->setScale(ARROW_SCALE);
	fields->m_prevArrow = CCMenuItemSpriteExtra::create(prevSprite, this, menu_selector(StatsGarageLayer::switchPage));
	fields->m_nextArrow = CCMenuItemSpriteExtra::create(nextSprite, this, menu_selector(StatsGarageLayer::switchPage));
	fields->m_nextArrow->setID("next-arrow"_spr);
	fields->m_prevArrow->setID("prev-arrow"_spr);
	
	fields->m_statsMenu->addChild(fields->m_prevArrow, INT_MIN, -1);
	fields->m_statsMenu->addChild(fields->m_nextArrow, INT_MAX, 1);

	schedule(schedule_selector(StatsGarageLayer::pageChildren));

	return true;
}

void StatsGarageLayer::switchPage(CCObject* sender) {
	auto fields = m_fields.self();
	if (!fields->m_statsMenu) return;
	CCNode* button = static_cast<CCNode*>(sender);
	if (fields->m_statsMenu->getChildrenCount() < 2) {
		fields->m_currentPage = 0;
		return;
	}
	if (fields->m_statsMenu->getChildrenCount() == 2) fields->m_requestedPage = 0;

	int actualChildren = fields->m_statsMenu->getChildrenCount() - 2;
	int pageDelta = button->getTag();
	int maxPage = (actualChildren - 1)/ELEMENTS_PER_PAGE;
	if (fields->m_currentPage + pageDelta < 0) fields->m_requestedPage = maxPage;
	else if (fields->m_currentPage + pageDelta > maxPage) fields->m_requestedPage = 0;
	else fields->m_requestedPage = fields->m_currentPage + pageDelta;
}

void StatsGarageLayer::pageChildren(float) {
	auto fields = m_fields.self();
	if (!fields->m_statsMenu) return;

	if (fields->m_statsMenu->getChildrenCount() < 2) return;
	int actualChildren = fields->m_statsMenu->getChildrenCount() - 2;
	if (
		fields->m_requestedPage == fields->m_currentPage &&
		actualChildren == fields->m_previousActualChildren.size()
	) {
		bool allSame = true;
		for (size_t i = 1; i < fields->m_statsMenu->getChildrenCount()-1; i++)
			if (fields->m_statsMenu->getChildByIndex(i) != fields->m_previousActualChildren[i-1])
				allSame = false;
		if (allSame) return;
	}

	fields->m_previousActualChildren.clear();
	fields->m_currentPage = fields->m_requestedPage;

	fields->m_prevArrow->setVisible(fields->m_statsMenu->getChildrenCount() > ELEMENTS_PER_PAGE + 2);
	fields->m_nextArrow->setVisible(fields->m_statsMenu->getChildrenCount() > ELEMENTS_PER_PAGE + 2);
	
	int maxPage = actualChildren == 0 ? 0 : (actualChildren - 1)/ELEMENTS_PER_PAGE;
	if (fields->m_currentPage > maxPage) fields->m_currentPage = maxPage; 
	for (size_t i = 1; i < fields->m_statsMenu->getChildrenCount()-1; i++) {
		CCNode* child = fields->m_statsMenu->getChildByIndex(i);
		fields->m_previousActualChildren.push_back(child);
		child->setVisible((i - 1)/ELEMENTS_PER_PAGE == fields->m_currentPage);
	}
	
	fields->m_statsMenu->updateLayout();
	
	auto winSize = CCDirector::get()->getWinSize();
	if (maxPage == 0 && actualChildren != 0) {
		CCNode* firstChild = fields->m_statsMenu->getChildByIndex(1);
		CCNode* firstLabel = firstChild->getChildByIndex(1);
		if (!firstLabel) return;

		float topLabelMaxY = firstChild->convertToWorldSpace(CCPoint{0, firstLabel->boundingBox().getMaxY()}).y;
		float newTopLabelMaxY = winSize.height - TOP_MARGIN;
		float topLabelMaxYDelta = topLabelMaxY - newTopLabelMaxY;
		fields->m_statsMenu->setPositionY(fields->m_statsMenu->getPositionY() - topLabelMaxYDelta);
	} else {
		fields->m_statsMenu->setPositionY(winSize.height - TOP_MARGIN);
	}
}