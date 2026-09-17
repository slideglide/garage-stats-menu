#include "StatsGarageLayer.hpp"
#include "../api/api.hpp"
#include "../utils.hpp"
#include <algorithm>

using namespace geode::prelude;

void StatsGarageLayer::onModify(auto& self) {
    if (!self.setHookPriority("GJGarageLayer::init", Priority::FirstPost)) {
        log::error("Failed to set hook priority for GJGarageLayer::init");
    }
}

bool StatsGarageLayer::init() {
    if (!GJGarageLayer::init()) return false;

    auto* fields = m_fields.self();
    const auto safeArea = geode::utils::getSafeAreaRect();

    fields->m_statsContainer = createStatsContainer(safeArea);
    this->addChild(fields->m_statsContainer);

    setupNavigation();
    rebuildStats();

    fields->m_statListener = StatChangeEvent().listen([this]() {
        this->rebuildStats();
    });

    return true;
}

CCNode* StatsGarageLayer::createStatsContainer(const CCRect& safeArea) {
    auto container = CCNode::create();
    container->setID("stats-container"_spr);
    container->setZOrder(2);
    container->ignoreAnchorPointForPosition(false);
    container->setAnchorPoint({1.f, 1.f});
    container->setContentSize({80.f, safeArea.size.height - 40.f});
    container->setPosition({safeArea.getMaxX() - RIGHT_MARGIN, safeArea.getMaxY() - TOP_MARGIN});
    container->setScale(getAdaptiveScale(safeArea));
    container->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setCrossAxisAlignment(AxisAlignment::End)
            ->setCrossAxisOverflow(true)
            ->setGap(ITEM_GAP)
            ->setAutoScale(false)
    );
    return container;
}

CCNode* StatsGarageLayer::createStatItemContainer(Label* label, CCNode* icon, std::string_view id) {
    auto container = CCNode::create();
    container->setID(fmt::format("{}-container", id));
    container->setContentSize({80.f, 15.f});
    container->setLayout(
        RowLayout::create()
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoScale(false)
            ->setGap(4.f)
    );
    container->addChild(label);
    container->addChild(icon);
    container->updateLayout();
    return container;
}

void StatsGarageLayer::addStatItem(
    std::vector<Ref<CCNode>>& target,
    std::string_view id,
    Ref<CCNode> icon,
    float scale,
    int number
) {
    if (!icon) return;

    if (icon->getParent()) {
        icon->removeFromParentAndCleanup(false);
    }

    icon->setID(fmt::format("{}-icon", id));
    icon->setScale(scale);

    auto label = Label::create(stats::utils::convertNumToAbbreviatedString(number), "bigFont.fnt");
    label->setID(fmt::format("{}-label", id));
    label->setScale(0.34f);

    target.push_back(createStatItemContainer(label, icon, id));
}

float StatsGarageLayer::getAdaptiveScale(const CCRect& safeArea) {
    #ifdef GEODE_IS_MOBILE
    constexpr float MIN_SCALE = 0.68f;
    constexpr float MAX_SCALE = 0.85f;
    #else
    constexpr float MIN_SCALE = 0.50f;
    constexpr float MAX_SCALE = 0.85f;
    #endif

    constexpr float MAX_CONTENT_HEIGHT = 215.f;
    constexpr float VERTICAL_PADDING = 35.f;

    const float availableHeight = std::max(100.f, safeArea.size.height - VERTICAL_PADDING);
    const float verticalFitScale = availableHeight / MAX_CONTENT_HEIGHT;

    const float baseScale = std::min(1.0f, verticalFitScale) * MAX_SCALE;

    const float aspectRatio = safeArea.size.width / std::max(1.0f, safeArea.size.height);
    float aspectFactor = 1.0f;
    if (aspectRatio < 1.6f) {
        aspectFactor = std::max(0.65f, aspectRatio / 1.6f);
    }

    const float calculatedScale = baseScale * aspectFactor;
    return std::clamp(calculatedScale, MIN_SCALE, MAX_SCALE);
}

void StatsGarageLayer::setupNavigation() {
    auto* fields = m_fields.self();

    auto createArrowContainer = [this](bool isNext) -> Ref<CCNode> {
        auto btn = Button::createWithSpriteFrameName("GJ_arrow_02_001.png", [this, isNext](auto) {
            this->goToPage(m_fields->m_currentPage + (isNext ? 1 : -1));
        });

        if (auto* sprite = static_cast<CCSprite*>(btn->getDisplayNode())) {
            sprite->setRotation(isNext ? -90.f : 90.f);
        }

        btn->setID("arrow-button"_spr);
        btn->setScale(ARROW_SCALE);

        auto container = CCNode::create();
        container->setID(isNext ? "next-arrow-container"_spr : "prev-arrow-container"_spr);
        container->setContentSize({80.f, 16.f});
        container->setLayout(
            RowLayout::create()
                ->setAxisAlignment(AxisAlignment::End)
                ->setAutoScale(false)
        );
        container->addChild(btn);
        container->updateLayout();

        return container;
    };

    fields->m_prevArrow = createArrowContainer(false);
    fields->m_nextArrow = createArrowContainer(true);
}

int StatsGarageLayer::getMaxPage() {
    auto fields = m_fields.self();
    const size_t total = fields->m_statNodes.size();
    return total == 0 ? 0 : static_cast<int>((total - 1) / ELEMENTS_PER_PAGE);
}

void StatsGarageLayer::goToPage(int page) {
    auto* fields = m_fields.self();
    fields->m_currentPage = std::clamp(page, 0, getMaxPage());
    renderCurrentPage();
}

void StatsGarageLayer::rebuildStats() {
    auto* fields = m_fields.self();
    if (!fields->m_statsContainer) return;

    if (fields->m_isRebuilding) return;
    fields->m_isRebuilding = true;

    fields->m_statNodes.clear();

    for (const auto& def : DEFAULT_STATS) {
        if (Mod::get()->getSettingValue<bool>(def.setting)) {
            auto* sprite = CCSprite::createWithSpriteFrameName(def.spriteFrame.c_str());
            const int num = GameStatsManager::sharedState()->getStat(def.statNum.c_str());
            addStatItem(fields->m_statNodes, def.id, sprite, def.scale, num);
        }
    }

    StatsManager::get()->forEachStat([this, fields](std::string_view id, StatItem& stat) {
        if (stat.provider) {
            if (auto* node = stat.provider()) {
                addStatItem(fields->m_statNodes, id, node, stat.nodeScale, stat.displayedNumber);
            }
        }
    });

    goToPage(fields->m_currentPage);
    fields->m_isRebuilding = false;
}

void StatsGarageLayer::renderCurrentPage() {
    auto* fields = m_fields.self();
    if (!fields->m_statsContainer) return;

    fields->m_statsContainer->removeAllChildren();

    const int total = static_cast<int>(fields->m_statNodes.size());
    const int maxPage = getMaxPage();
    const bool needsPagination = total > ELEMENTS_PER_PAGE;

    auto updateArrowState = [](CCNode* container, bool enabled) {
        if (!container) return;

        if (auto* btn = container->getChildByType<Button>()) {
            btn->setEnabled(true);
            btn->setOpacity(enabled ? 255 : 100);
        }
    };

    if (needsPagination) {
        updateArrowState(fields->m_prevArrow, fields->m_currentPage > 0);
        fields->m_statsContainer->addChild(fields->m_prevArrow);
    }

    const size_t start = static_cast<size_t>(fields->m_currentPage) * ELEMENTS_PER_PAGE;
    const size_t end = std::min(start + ELEMENTS_PER_PAGE, fields->m_statNodes.size());

    for (size_t i = start; i < end; ++i) {
        fields->m_statsContainer->addChild(fields->m_statNodes[i]);
    }

    if (needsPagination) {
        updateArrowState(fields->m_nextArrow, fields->m_currentPage < maxPage);
        fields->m_statsContainer->addChild(fields->m_nextArrow);
    }

    fields->m_statsContainer->updateLayout();
}
