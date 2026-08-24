#include <Geode/modify/GJGarageLayer.hpp>
#include <Geode/ui/Button.hpp>
#include <algorithm>
#include <array>
#include <string_view>
#include <vector>

#include "../api/api.hpp"
#include "../utils.hpp"

using namespace geode::prelude;

constexpr int ELEMENTS_PER_PAGE = 10;
constexpr float TOP_MARGIN = 12.f;
constexpr float RIGHT_MARGIN = 18.f;
constexpr float ITEM_GAP = 3.f;
constexpr float ARROW_GAP = 4.f;
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
        CCNode* m_statsContainer = nullptr;
        CCNode* m_arrowContainer = nullptr;
        Button* m_prevArrow = nullptr;
        Button* m_nextArrow = nullptr;
        std::vector<Ref<CCNode>> m_allStatNodes;
        int m_currentPage = 0;
        int m_requestedPage = 0;
        int m_maxPage = 0;
    };

    void layoutPage() {
        auto* fields = m_fields.self();
        auto& all = fields->m_allStatNodes;

        const int actual = static_cast<int>(all.size());
        fields->m_maxPage = actual <= 0 ? 0 : (actual - 1) / ELEMENTS_PER_PAGE;
        fields->m_currentPage = std::clamp(fields->m_requestedPage, 0, fields->m_maxPage);
        fields->m_requestedPage = fields->m_currentPage;

        const bool hasMultiplePages = actual > ELEMENTS_PER_PAGE;

        fields->m_statsContainer->removeAllChildrenWithCleanup(false);

        const int start = fields->m_currentPage * ELEMENTS_PER_PAGE;
        const int end = std::min(start + ELEMENTS_PER_PAGE, actual);

        for (int i = start; i < end; ++i) {
            fields->m_statsContainer->addChild(all[i]);
        }

        fields->m_statsContainer->updateLayout();

        fields->m_arrowContainer->setVisible(hasMultiplePages);
        fields->m_prevArrow->setEnabled(hasMultiplePages && fields->m_currentPage > 0);
        fields->m_prevArrow->setOpacity(fields->m_currentPage > 0 ? 255 : 100);
        fields->m_nextArrow->setEnabled(hasMultiplePages && fields->m_currentPage < fields->m_maxPage);
        fields->m_nextArrow->setOpacity(fields->m_currentPage < fields->m_maxPage ? 255 : 100);
    }

    void setupArrows() {
        auto* fields = m_fields.self();

        auto createArrowBtn = [this, fields](bool isNext) {
            auto* spr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
            if (isNext) {
                spr->setFlipX(true);
            }

            auto* btn = Button::createWithNode(
                spr,
                [this, fields, isNext](auto) {
                    fields->m_requestedPage = isNext
                        ? std::min(fields->m_currentPage + 1, fields->m_maxPage)
                        : std::max(fields->m_currentPage - 1, 0);
                    this->layoutPage();
                }
            );

            btn->setScale(ARROW_SCALE);
            btn->setRotation(90.f);
            return btn;
        };

        fields->m_prevArrow = createArrowBtn(false);
        fields->m_prevArrow->setID("prev-arrow"_spr);

        fields->m_nextArrow = createArrowBtn(true);
        fields->m_nextArrow->setID("next-arrow"_spr);

        // Menuless Button layout container
        fields->m_arrowContainer = CCNode::create();
        fields->m_arrowContainer->setID("arrow-container"_spr);
        fields->m_arrowContainer->setAnchorPoint({1.f, 1.f});
        fields->m_arrowContainer->setContentSize({30.f, 40.f});

        fields->m_arrowContainer->setLayout(
            ColumnLayout::create()
                ->setAxisReverse(true)
                ->setGap(ARROW_GAP)
                ->setAutoScale(false)
        );
        fields->m_arrowContainer->addChild(fields->m_prevArrow);
        fields->m_arrowContainer->addChild(fields->m_nextArrow);
        fields->m_arrowContainer->updateLayout();

        const auto safeArea = geode::utils::getSafeAreaRect();
        fields->m_arrowContainer->setPosition({safeArea.getMaxX() - RIGHT_MARGIN, safeArea.getMaxY() - TOP_MARGIN});
        this->addChild(fields->m_arrowContainer);
    }

    static void addStatItem(std::vector<Ref<CCNode>>& target, std::string_view id, CCNode* icon, float scale, int number) {
        if (!icon) return;

        if (icon->getParent()) {
            icon->removeFromParentAndCleanup(false);
        }

        icon->setID(fmt::format("{}-icon", id));
        icon->setScale(scale);

        const std::string displayStr = stats::utils::convertNumToAbbreviatedString(number);
        auto* label = Label::create(displayStr, "bigFont.fnt");
        label->setID(fmt::format("{}-label", id));
        label->setScale(0.34f);

        auto* container = CCNode::create();
        container->setID(fmt::format("{}-container", id));
        container->setContentSize({80.f, 15.f});
        container->setAnchorPoint({1.f, 0.5f});

        container->setLayout(
            RowLayout::create()
                ->setAxisAlignment(AxisAlignment::End)
                ->setAutoScale(false)
                ->setGap(4.f)
        );

        container->addChild(label);
        container->addChild(icon);
        container->updateLayout();

        target.push_back(container);
    }

    bool init() {
        if (!GJGarageLayer::init()) {
            return false;
        }

        auto* fields = m_fields.self();

        for (const auto& def : DEFAULT_STATS) {
            if (Mod::get()->getSettingValue<bool>(def.setting)) {
                auto* sprite = CCSprite::createWithSpriteFrameName(def.spriteFrame.c_str());
                const int num = GameStatsManager::sharedState()->getStat(def.statNum.c_str());
                addStatItem(fields->m_allStatNodes, def.id, sprite, def.scale, num);
            }
        }

        for (const auto& [id, stat] : StatsManager::get()->getManagedStats()) {
            if (auto node = stat.displayNode.lock()) {
                addStatItem(fields->m_allStatNodes, id, node, stat.nodeScale, stat.displayedNumber);
            }
        }

        const auto safeArea = geode::utils::getSafeAreaRect();

        fields->m_statsContainer = CCNode::create();
        fields->m_statsContainer->setID("stats-container"_spr);
        fields->m_statsContainer->setZOrder(2);
        fields->m_statsContainer->setAnchorPoint({1.f, 1.f});
        fields->m_statsContainer->setContentSize({100.f, safeArea.size.height - 40.f});
        fields->m_statsContainer->setPosition({safeArea.getMaxX() - RIGHT_MARGIN, safeArea.getMaxY() - TOP_MARGIN});

        fields->m_statsContainer->setLayout(
            ColumnLayout::create()
                ->setAxisReverse(true)
                ->setAxisAlignment(AxisAlignment::End)
                ->setCrossAxisAlignment(AxisAlignment::End)
                ->setCrossAxisOverflow(true)
                ->setGap(ITEM_GAP)
                ->setAutoScale(true)
        );
        this->addChild(fields->m_statsContainer);

        setupArrows();
        layoutPage();

        return true;
    }
};