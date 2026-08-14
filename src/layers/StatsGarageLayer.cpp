#include "StatsGarageLayer.hpp"
#include <StatsDisplayAPI.h>

using namespace geode::prelude;

constexpr int ELEMENTS_PER_PAGE = 6; 
constexpr float TOP_MARGIN = 12.f;
constexpr float RIGHT_MARGIN = 18.f;
constexpr float ITEM_STEP_Y = 15.f;
constexpr float ARROW_SCALE = 0.5f;

static void addDefaultItem(CCNode* statsMenu,
                           std::string setting,
                           std::string id,
                           ZStringView spriteFrameName,
                           ZStringView statNum,
                           float scale) {
    if (Mod::get()->getSettingValue<bool>(setting)) {
        statsMenu->addChild(StatsDisplayAPI::getNewItem(
            id, CCSprite::createWithSpriteFrameName(spriteFrameName.c_str()),
            GameStatsManager::sharedState()->getStat(statNum.c_str()), scale));
    }
}

static bool isArrowNode(StatsGarageLayer::Fields* fields, CCNode* node) {
    return node == fields->m_prevArrow || node == fields->m_nextArrow;
}

static int getActualChildrenCount(StatsGarageLayer::Fields* fields, CCNode* statsMenu) {
    if (!statsMenu)
        return 0;

    int actualChildren = 0;
    for (CCNode* child : statsMenu->getChildrenExt()) {
        if (isArrowNode(fields, child))
            continue;
        ++actualChildren;
    }

    return actualChildren;
}

static int getMaxPage(int actualChildren) {
    return actualChildren <= 0 ? 0 : (actualChildren - 1) / ELEMENTS_PER_PAGE;
}

static void requestPage(StatsGarageLayer::Fields* fields, int pageDelta) {
    if (!fields->m_statsMenu)
        return;
    if (fields->m_statsMenu->getChildrenCount() < 2) {
        fields->m_currentPage = 0;
        return;
    }
    if (fields->m_statsMenu->getChildrenCount() == 2)
        fields->m_requestedPage = 0;

    int actualChildren = getActualChildrenCount(fields, fields->m_statsMenu);
    int maxPage = getMaxPage(actualChildren);
    if (fields->m_currentPage + pageDelta < 0) {
        fields->m_requestedPage = maxPage;
    } else if (fields->m_currentPage + pageDelta > maxPage) {
        fields->m_requestedPage = 0;
    } else {
        fields->m_requestedPage = fields->m_currentPage + pageDelta;
    }
}

bool StatsGarageLayer::init() {
    if (!GJGarageLayer::init())
        return false;

    auto fields = m_fields.self();

    fields->m_statsMenu = CCMenu::create();
    fields->m_statsMenu->setID("stats-menu"_spr);
    fields->m_statsMenu->setZOrder(2);
    fields->m_statsMenu->setPosition({0.f, 0.f});

    this->addChild(fields->m_statsMenu);

    struct DefaultStatItem {
        std::string setting;
        std::string id;
        std::string spriteFrameName;
        std::string statNum;
        float scale;
    };

    const DefaultStatItem defaultItems[] = {
        {"stars-stat", "stars", "GJ_starsIcon_001.png", "6", 0.54f},
        {"moons-stat", "moons", "GJ_moonsIcon_001.png", "28", 0.54f},
        {"gold-coins-stat", "coins", "GJ_coinsIcon_001.png", "8", 0.51f},
        {"user-coins-stat", "user-coins", "GJ_coinsIcon2_001.png", "12", 0.51f},
        {"orbs-stat", "orbs", "currencyOrbIcon_001.png", "14", 0.54f},
        {"diamonds-stat", "diamonds", "GJ_diamondsIcon_001.png", "13", 0.6f},
        {"diamond-shards-stat", "diamond-shards", "currencyDiamondIcon_001.png", "29", 0.54f},
    };

    for (const auto& item : defaultItems) {
        addDefaultItem(fields->m_statsMenu, item.setting, item.id, item.spriteFrameName,
                       item.statNum, item.scale);
    }

    setupArrows();
    schedule(schedule_selector(StatsGarageLayer::pageChildren));

    return true;
}

void StatsGarageLayer::setupArrows() {
    auto fields = m_fields.self();

    fields->m_prevArrow =
        Button::createWithSpriteFrameName("GJ_arrow_02_001.png", [fields](auto) {
            requestPage(fields, -1);
        });
    auto prevSprite = static_cast<CCSprite*>(fields->m_prevArrow->getDisplayNode());
    prevSprite->setRotation(90);
    prevSprite->setScale(ARROW_SCALE);
    fields->m_prevArrow->setID("prev-arrow"_spr);

    fields->m_statsMenu->addChild(fields->m_prevArrow, -1, -1);

    fields->m_nextArrow =
        Button::createWithSpriteFrameName("GJ_arrow_02_001.png", [fields](auto) {
            requestPage(fields, 1);
        });
    auto nextSprite = static_cast<CCSprite*>(fields->m_nextArrow->getDisplayNode());
    nextSprite->setFlipX(true);
    nextSprite->setRotation(90);
    nextSprite->setScale(ARROW_SCALE);
    fields->m_nextArrow->setID("next-arrow"_spr);

    fields->m_statsMenu->addChild(fields->m_nextArrow, 1, 1);
}

void StatsGarageLayer::pageChildren(float dt) {
    auto fields = m_fields.self();
    auto statsMenu = fields->m_statsMenu;

    if (!statsMenu)
        return;

    auto children = statsMenu->getChildrenExt();
    if (children.size() < 2)
        return;

    int actualChildren = getActualChildrenCount(fields, statsMenu);

    fields->m_previousActualChildren.clear();
    fields->m_currentPage = fields->m_requestedPage;

    bool hasMultiplePages = actualChildren > ELEMENTS_PER_PAGE;
    fields->m_prevArrow->setVisible(hasMultiplePages);
    fields->m_nextArrow->setVisible(hasMultiplePages);

    int maxPage = getMaxPage(actualChildren);
    if (fields->m_currentPage > maxPage)
        fields->m_currentPage = maxPage;

    auto safeArea = geode::utils::getSafeAreaRect();
    float xPos = safeArea.getMaxX() - RIGHT_MARGIN;
    float safeTop = safeArea.getMaxY();

    float startY = hasMultiplePages ? (safeTop - 34.f) : (safeTop - TOP_MARGIN);

    int actualIndex = 0;
    int visibleIndex = 0;

    for (CCNode* child : children) {
        if (isArrowNode(fields, child))
            continue;

        bool isVisible = (actualIndex / ELEMENTS_PER_PAGE == fields->m_currentPage);
        child->setVisible(isVisible);

        if (isVisible) {
            fields->m_previousActualChildren.push_back(child);
            child->setPosition({xPos, startY - (visibleIndex * ITEM_STEP_Y)});
            visibleIndex++;
        } else {
            child->setPosition({-9999.f, -9999.f});
        }
        ++actualIndex;
    }

    if (hasMultiplePages && visibleIndex > 0) {
        float lastItemY = startY - ((visibleIndex - 1) * ITEM_STEP_Y);

        fields->m_prevArrow->setPosition({xPos, safeTop - 15.f});
        fields->m_nextArrow->setPosition({xPos, lastItemY - 16.f});
    }
}
