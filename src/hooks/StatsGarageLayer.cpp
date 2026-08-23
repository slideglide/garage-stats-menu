#include <Geode/modify/GJGarageLayer.hpp>
#include <Geode/ui/Button.hpp>
#include "../api/api.hpp"

using namespace geode::prelude;

constexpr int ELEMENTS_PER_PAGE = 6;
constexpr float TOP_MARGIN = 12.f;
constexpr float RIGHT_MARGIN = 18.f;
constexpr float ITEM_STEP_Y = 15.f;
constexpr float ARROW_SCALE = 0.5f;

static bool isArrowNode(CCMenuItemSpriteExtra* prevArrow, CCMenuItemSpriteExtra* nextArrow, CCNode* node) {
    return node == prevArrow || node == nextArrow;
}

static int getActualChildrenCount(CCMenuItemSpriteExtra* prevArrow, CCMenuItemSpriteExtra* nextArrow, CCNode* statsMenu) {
    if (!statsMenu) {
        return 0;
    }

    int count = 0;
    for (CCNode* child : statsMenu->getChildrenExt()) {
        if (isArrowNode(prevArrow, nextArrow, child)) {
            continue;
        }
        ++count;
    }
    return count;
}

static int getMaxPage(int actualChildren) { return actualChildren <= 0 ? 0 : (actualChildren - 1) / ELEMENTS_PER_PAGE; }

class $modify(StatsGarageLayer, GJGarageLayer) {
    static void onModify(auto& self) {
        if (!self.setHookPriority("GJGarageLayer::init", geode::Priority::FirstPost)) {
            geode::log::error("Failed to set hook priority for GJGarageLayer::init");
        }
    }

    struct Fields {
        CCMenu* m_statsMenu = nullptr;
        int m_currentPage = 0;
        int m_requestedPage = 0;
        CCMenuItemSpriteExtra* m_prevArrow = nullptr;
        CCMenuItemSpriteExtra* m_nextArrow = nullptr;
    };

    void setupArrows() {
        auto fields = m_fields.self();

        fields->m_prevArrow = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_02_001.png", ARROW_SCALE, [fields](auto) {
            if (!fields->m_statsMenu) {
                return;
            }
            
            if (fields->m_statsMenu->getChildrenCount() < 2) {
                fields->m_currentPage = 0;
                return;
            }

            int actual = getActualChildrenCount(fields->m_prevArrow, fields->m_nextArrow, fields->m_statsMenu);
            int maxPage = getMaxPage(actual);
            if (fields->m_currentPage - 1 < 0) {
                fields->m_requestedPage = maxPage;
            } else {
                fields->m_requestedPage = fields->m_currentPage - 1;
            }
        });
        fields->m_prevArrow->setRotation(90);
        fields->m_prevArrow->setID("prev-arrow"_spr);

        fields->m_statsMenu->addChild(fields->m_prevArrow, -1, -1);

        fields->m_nextArrow = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_02_001.png", ARROW_SCALE, [fields](auto) {
            if (!fields->m_statsMenu) {
                return;
            }
            if (fields->m_statsMenu->getChildrenCount() < 2) {
                fields->m_currentPage = 0;
                return;
            }
            int actual = getActualChildrenCount(fields->m_prevArrow, fields->m_nextArrow, fields->m_statsMenu);
            int maxPage = getMaxPage(actual);
            if (fields->m_currentPage + 1 > maxPage) {
                fields->m_requestedPage = 0;
            } else {
                fields->m_requestedPage = fields->m_currentPage + 1;
            }
        });
        static_cast<CCSprite*>(fields->m_nextArrow->getNormalImage())->setFlipX(true);
        fields->m_nextArrow->setRotation(90);
        fields->m_nextArrow->setScale(ARROW_SCALE);
        fields->m_nextArrow->setID("next-arrow"_spr);

        fields->m_statsMenu->addChild(fields->m_nextArrow, 1, 1);
    }

    void pageChildren(float dt) {
        auto fields = m_fields.self();
        auto statsMenu = fields->m_statsMenu;

        if (!statsMenu) {
            return;
        }

        auto children = statsMenu->getChildrenExt();
        if (children.size() < 2) {
            return;
        }

        int actualChildren = getActualChildrenCount(fields->m_prevArrow, fields->m_nextArrow, statsMenu);

        fields->m_currentPage = fields->m_requestedPage;

        bool hasMultiplePages = actualChildren > ELEMENTS_PER_PAGE;
        fields->m_prevArrow->setVisible(hasMultiplePages);
        fields->m_nextArrow->setVisible(hasMultiplePages);

        int maxPage = getMaxPage(actualChildren);
        if (fields->m_currentPage > maxPage) {
            fields->m_currentPage = maxPage;
        }

        auto safeArea = geode::utils::getSafeAreaRect();
        float xPos = safeArea.getMaxX() - RIGHT_MARGIN;
        float safeTop = safeArea.getMaxY();

        float startY = hasMultiplePages ? (safeTop - 34.f) : (safeTop - TOP_MARGIN);

        int actualIndex = 0;
        int visibleIndex = 0;

        for (CCNode* child : children) {
            if (isArrowNode(fields->m_prevArrow, fields->m_nextArrow, child)) {
                continue;
            }

            bool isVisible = (actualIndex / ELEMENTS_PER_PAGE == fields->m_currentPage);
            child->setVisible(isVisible);

            if (isVisible) {
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

    static void addStatItem(CCMenu* menu, const std::string& id, CCNode* icon, float iconScale, int number) {
        auto* container = CCMenu::create();
        container->setID(id + "-container");
        container->setContentSize({0, 0});

        if (icon != nullptr) {
            icon->setID(id + "-icon");
            icon->setScale(iconScale);
            if (icon->getParent() != nullptr) {
                icon->removeFromParentAndCleanup(false);
            }
            icon->setPosition({0, 0});
            container->addChild(icon);
        }

        auto* label = geode::Label::create(fmt::to_string(number), "bigFont.fnt");
        label->setID(id + "-label");
        label->setScale(0.34f);
        label->setAnchorPoint({1.0f, 0.5f});
        label->setPosition({-12.0f, 0.5f});
        container->addChild(label);

        menu->addChild(container);
    }

    bool init() {
        if (!GJGarageLayer::init()) {
            return false;
        }

        auto fields = m_fields.self();

        fields->m_statsMenu = CCMenu::create();
        fields->m_statsMenu->setID("stats-menu"_spr);
        fields->m_statsMenu->setZOrder(2);
        fields->m_statsMenu->setPosition({0.f, 0.f});
        this->addChild(fields->m_statsMenu);

        struct DefaultStat {
            std::string setting;
            std::string id;
            ZStringView spriteFrame;
            ZStringView statNum;
            float scale;
        };

        const std::array<DefaultStat, 7> defaults = {{
            {"stars-stat", "stars", "GJ_starsIcon_001.png", "6", 0.54f},
            {"moons-stat", "moons", "GJ_moonsIcon_001.png", "28", 0.54f},
            {"gold-coins-stat", "coins", "GJ_coinsIcon_001.png", "8", 0.51f},
            {"user-coins-stat", "user-coins", "GJ_coinsIcon2_001.png", "12", 0.51f},
            {"orbs-stat", "orbs", "currencyOrbIcon_001.png", "14", 0.54f},
            {"diamonds-stat", "diamonds", "GJ_diamondsIcon_001.png", "13", 0.6f},
            {"diamond-shards-stat", "diamond-shards", "currencyDiamondIcon_001.png", "29", 0.54f},
        }};

        for (const auto& def : defaults) {
            if (Mod::get()->getSettingValue<bool>(def.setting)) {
                auto* sprite = CCSprite::createWithSpriteFrameName(def.spriteFrame.c_str());
                int num = GameStatsManager::sharedState()->getStat(def.statNum.c_str());
                addStatItem(fields->m_statsMenu, def.id, sprite, def.scale, num);
            }
        }

        auto managedStats = StatsManager::get()->getManagedStats();
        for (auto const& [id, stat] : managedStats) {
            auto node = stat.displayNode.lock();
            addStatItem(fields->m_statsMenu, id, node, stat.nodeScale, stat.displayedNumber);
        }

        setupArrows();
        schedule(schedule_selector(StatsGarageLayer::pageChildren));

        return true;
    }
};
