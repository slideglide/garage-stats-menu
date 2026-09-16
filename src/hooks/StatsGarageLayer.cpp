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
    if (!GJGarageLayer::init()) {
        return false;
    }

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
    Ref container = CCNode::create();
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
        ->setAutoScale(true)
    );
    return container;
}

CCNode* StatsGarageLayer::createStatItemContainer(CCNode* label, CCNode* icon, std::string_view id) {
    Ref container = CCNode::create();
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
    return container;
}

void StatsGarageLayer::addStatItem(
    std::vector<Ref<CCNode>>& target,
    std::string_view id,
    CCNode* icon,
    float scale,
    int number
) {
    if (!icon) return;

    if (icon->getParent()) {
        icon->removeFromParentAndCleanup(false);
    }

    icon->setID(fmt::format("{}-icon", id));
    icon->setScale(scale);

    Ref label = Label::create(stats::utils::convertNumToAbbreviatedString(number), "bigFont.fnt");
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

void StatsGarageLayer::setArrowState(Button* button, bool enabled) {
    if (!button) return;
    button->setEnabled(enabled);
    button->setOpacity(enabled ? 255 : 100);
}

void StatsGarageLayer::setupNavigation() {
    auto* fields = m_fields.self();

    auto createArrowContainer = [this, fields](bool isNext) -> std::pair<Ref<CCNode>, Ref<Button>> {
        Ref btn = Button::createWithSpriteFrameName("GJ_arrow_02_001.png", [this, fields, isNext](auto) {
            fields->m_requestedPage = std::clamp(
                fields->m_currentPage + (isNext ? 1 : -1),
                                                 0,
                                                 fields->m_maxPage
            );
            this->applyPagination();
        });

        if (auto* sprite = static_cast<CCSprite*>(btn->getDisplayNode())) {
            sprite->setFlipX(isNext);
            sprite->setRotation(90.f);
        }

        btn->setID(isNext ? "next-arrow"_spr : "prev-arrow"_spr);
        btn->setScale(ARROW_SCALE);

        Ref container = CCNode::create();
        container->setID(isNext ? "next-arrow-container"_spr : "prev-arrow-container"_spr);
        container->setContentSize({80.f, 16.f});
        container->setAnchorPoint({1.f, 0.5f});
        container->setLayoutOptions(AxisLayoutOptions::create()->setAutoScale(false));
        container->setLayout(
            RowLayout::create()
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoScale(false)
        );
        container->addChild(btn);
        container->updateLayout();

        return {container, btn};
    };

    std::tie(fields->m_prevArrowContainer, fields->m_prevArrow) = createArrowContainer(false);
    std::tie(fields->m_nextArrowContainer, fields->m_nextArrow) = createArrowContainer(true);
}

void StatsGarageLayer::rebuildStats() {
    auto* fields = m_fields.self();
    if (!fields->m_statsContainer) return;

    if (fields->m_isRebuilding) return;
    fields->m_isRebuilding = true;

    const int preservedPage = fields->m_currentPage;

    for (auto& container : fields->m_statNodes) {
        if (container) {
            for (auto* child : container->getChildrenExt()) {
                if (child->getID().view().ends_with("-icon")) {
                    child->removeFromParentAndCleanup(false);
                }
            }
        }
    }

    fields->m_statNodes.clear();
    fields->m_statsContainer->removeAllChildren();

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

    fields->m_statsContainer->addChild(fields->m_prevArrowContainer);
    for (auto& node : fields->m_statNodes) {
        fields->m_statsContainer->addChild(node);
    }
    fields->m_statsContainer->addChild(fields->m_nextArrowContainer);

    fields->m_requestedPage = preservedPage;
    applyPagination();

    fields->m_isRebuilding = false;
}

void StatsGarageLayer::applyPagination() {
    auto* fields = m_fields.self();
    if (!fields->m_statsContainer) return;

    const auto& nodes = fields->m_statNodes;
    const int total = static_cast<int>(nodes.size());

    fields->m_maxPage = total <= 0 ? 0 : (total - 1) / ELEMENTS_PER_PAGE;
    fields->m_currentPage = std::clamp(fields->m_requestedPage, 0, fields->m_maxPage);
    fields->m_requestedPage = fields->m_currentPage;

    const bool hasPages = total > ELEMENTS_PER_PAGE;
    const std::size_t start = static_cast<std::size_t>(fields->m_currentPage) * ELEMENTS_PER_PAGE;
    const std::size_t end = std::min(start + static_cast<std::size_t>(ELEMENTS_PER_PAGE), nodes.size());

    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (auto& node = nodes[i]) {
            node->setVisible(i >= start && i < end);
        }
    }

    fields->m_prevArrowContainer->setVisible(hasPages);
    fields->m_nextArrowContainer->setVisible(hasPages);

    setArrowState(fields->m_prevArrow, hasPages && fields->m_currentPage > 0);
    setArrowState(fields->m_nextArrow, hasPages && fields->m_currentPage < fields->m_maxPage);

    fields->m_statsContainer->updateLayout();
}
