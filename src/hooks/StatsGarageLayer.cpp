#include <Geode/modify/GJGarageLayer.hpp>
#include <Geode/ui/Button.hpp>
#include <array>
#include <string_view>
#include "../api/api.hpp"

using namespace geode::prelude;

constexpr int ELEMENTS_PER_PAGE = 6;
constexpr float TOP_MARGIN = 12.f;
constexpr float RIGHT_MARGIN = 18.f;
constexpr float ITEM_STEP_Y = 15.f;
constexpr float ARROW_SCALE = 0.5f;

struct DefaultStat {
    std::string_view setting;
    std::string_view id;
    ZStringView spriteFrame;
    ZStringView statNum;
    float scale;
};

static const auto DEFAULT_STATS = std::to_array<DefaultStat>({
    {"stars-stat", "stars", "GJ_starsIcon_001.png", "6", 0.54f},
    {"moons-stat", "moons", "GJ_moonsIcon_001.png", "28", 0.54f},
    {"gold-coins-stat", "coins", "GJ_coinsIcon_001.png", "8", 0.51f},
    {"user-coins-stat", "user-coins", "GJ_coinsIcon2_001.png", "12", 0.51f},
    {"orbs-stat", "orbs", "currencyOrbIcon_001.png", "14", 0.54f},
    {"diamonds-stat", "diamonds", "GJ_diamondsIcon_001.png", "13", 0.6f},
    {"diamond-shards-stat", "diamond-shards", "currencyDiamondIcon_001.png", "29", 0.54f},
});

class $modify(StatsGarageLayer, GJGarageLayer) {
    static void onModify(auto& self) {
        if (!self.setHookPriority("GJGarageLayer::init", geode::Priority::FirstPost)) {
            geode::log::error("Failed to set hook priority for GJGarageLayer::init");
        }
    }

    struct Fields {
        CCMenu* m_statsMenu = nullptr;
        CCMenuItemSpriteExtra* m_prevArrow = nullptr;
        CCMenuItemSpriteExtra* m_nextArrow = nullptr;
        int m_currentPage = 0;
        int m_requestedPage = 0;
    };

    void setupArrows() {
        auto* fields = m_fields.self();

        auto createArrow = [this, fields](bool isNext) {
            auto* arrow = CCMenuItemExt::createSpriteExtraWithFrameName(
                "GJ_arrow_02_001.png", ARROW_SCALE, [fields, isNext](auto) {
                    if (!fields->m_statsMenu || fields->m_statsMenu->getChildrenCount() < 2) {
                        fields->m_currentPage = 0;
                        return;
                    }

                    int actual = 0;
                    for (CCNode* child : fields->m_statsMenu->getChildrenExt()) {
                        if (child != fields->m_prevArrow && child != fields->m_nextArrow) {
                            ++actual;
                        }
                    }

                    const int maxPage = actual <= 0 ? 0 : (actual - 1) / ELEMENTS_PER_PAGE;

                    if (isNext) {
                        fields->m_requestedPage = (fields->m_currentPage + 1 > maxPage) ? 0 : fields->m_currentPage + 1;
                    } else {
                        fields->m_requestedPage = (fields->m_currentPage - 1 < 0) ? maxPage : fields->m_currentPage - 1;
                    }
                });

            arrow->setRotation(90.f);
            return arrow;
        };

        fields->m_prevArrow = createArrow(false);
        fields->m_prevArrow->setID("prev-arrow"_spr);
        fields->m_statsMenu->addChild(fields->m_prevArrow, -1, -1);

        fields->m_nextArrow = createArrow(true);
        if (auto* sprite = static_cast<CCSprite*>(fields->m_nextArrow->getNormalImage())) {
            sprite->setFlipX(true);
        }
        fields->m_nextArrow->setID("next-arrow"_spr);
        fields->m_statsMenu->addChild(fields->m_nextArrow, 1, 1);
    }

    void pageChildren(float dt) {
        auto* fields = m_fields.self();
        auto* statsMenu = fields->m_statsMenu;

        if (!statsMenu) {
            return;
        }

        auto children = statsMenu->getChildrenExt();
        if (children.size() < 2) {
            return;
        }

        auto isArrow = [fields](const CCNode* node) {
            return node == fields->m_prevArrow || node == fields->m_nextArrow;
        };

        int actualChildren = 0;
        for (CCNode* child : children) {
            if (!isArrow(child)) {
                ++actualChildren;
            }
        }

        const bool hasMultiplePages = actualChildren > ELEMENTS_PER_PAGE;
        const int maxPage = actualChildren <= 0 ? 0 : (actualChildren - 1) / ELEMENTS_PER_PAGE;

        fields->m_currentPage = std::min(fields->m_requestedPage, maxPage);
        fields->m_prevArrow->setVisible(hasMultiplePages);
        fields->m_nextArrow->setVisible(hasMultiplePages);

        const auto safeArea = geode::utils::getSafeAreaRect();
        const float xPos = safeArea.getMaxX() - RIGHT_MARGIN;
        const float safeTop = safeArea.getMaxY();
        const float startY = hasMultiplePages ? (safeTop - 34.f) : (safeTop - TOP_MARGIN);

        int actualIndex = 0;
        int visibleIndex = 0;

        auto updateNodeVisibility = [&](CCNode* child) {
            const bool isVisible = (actualIndex / ELEMENTS_PER_PAGE == fields->m_currentPage);
            child->setVisible(isVisible);

            if (isVisible) {
                child->setPosition({xPos, startY - (visibleIndex * ITEM_STEP_Y)});
                visibleIndex++;
            } else {
                child->setPosition({-9999.f, -9999.f});
            }
            ++actualIndex;
        };

        for (CCNode* child : children) {
            if (!isArrow(child)) {
                updateNodeVisibility(child);
            }
        }

        auto updateArrowPositions = [&]() {
            if (hasMultiplePages && visibleIndex > 0) {
                const float lastItemY = startY - ((visibleIndex - 1) * ITEM_STEP_Y);
                fields->m_prevArrow->setPosition({xPos, safeTop - 15.f});
                fields->m_nextArrow->setPosition({xPos, lastItemY - 16.f});
            }
        };

        updateArrowPositions();
    }

    static void addStatItem(CCMenu* menu, std::string_view id, CCNode* icon, float iconScale, int number) {
        auto* container = CCMenu::create();
        container->setID(fmt::format("{}-container", id));
        container->setContentSize({0.f, 0.f});

        if (icon) {
            icon->setID(fmt::format("{}-icon", id));
            icon->setScale(iconScale);
            if (icon->getParent()) {
                icon->removeFromParentAndCleanup(false);
            }
            icon->setPosition({0.f, 0.f});
            container->addChild(icon);
        }

        auto* label = geode::Label::create(fmt::to_string(number), "bigFont.fnt");
        label->setID(fmt::format("{}-label", id));
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

        auto* fields = m_fields.self();

        fields->m_statsMenu = CCMenu::create();
        fields->m_statsMenu->setID("stats-menu"_spr);
        fields->m_statsMenu->setZOrder(2);
        fields->m_statsMenu->setPosition({0.f, 0.f});
        this->addChild(fields->m_statsMenu);

        for (const auto& def : DEFAULT_STATS) {
            if (Mod::get()->getSettingValue<bool>(def.setting)) {
                auto* sprite = CCSprite::createWithSpriteFrameName(def.spriteFrame.c_str());
                const int num = GameStatsManager::sharedState()->getStat(def.statNum.c_str());
                addStatItem(fields->m_statsMenu, def.id, sprite, def.scale, num);
            }
        }

        for (const auto& [id, stat] : StatsManager::get()->getManagedStats()) {
            auto node = stat.displayNode.lock();
            addStatItem(fields->m_statsMenu, id, node, stat.nodeScale, stat.displayedNumber);
        }

        setupArrows();
        this->schedule(schedule_selector(StatsGarageLayer::pageChildren));

        return true;
    }
};
