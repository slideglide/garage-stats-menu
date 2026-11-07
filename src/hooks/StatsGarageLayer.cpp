#include "StatsGarageLayer.hpp"
#include <Geode/binding/GameStatsManager.hpp>
#include "../api/api.hpp"
#include "../utils.hpp"
#include <algorithm>

using namespace geode::prelude;

void StatsGarageLayer::onModify(auto& self) {
    if (!self.setHookPriorityPost("GJGarageLayer::init", Priority::First)) {
        log::error("Failed to set hook priority for GJGarageLayer::init");
    }
}

bool StatsGarageLayer::init() {
    if (!GJGarageLayer::init()) return false;

    auto* fields = m_fields.self();
    auto const safeArea = geode::utils::getSafeAreaRect();

    fields->m_availableHeight = (safeArea.size.height - VERTICAL_PADDING) / STATS_SCALE;
    fields->m_statsContainer = createStatsContainer(safeArea, fields->m_availableHeight);
    this->addChild(fields->m_statsContainer);

    setupNavigation();
    rebuildStats();

    fields->m_statListener = StatChangeEvent().listen([this]() {
        this->rebuildStats();
    });

    return true;
}

CCNode* StatsGarageLayer::createStatsContainer(CCRect const& safeArea, float availableHeight) {
    auto container = CCNode::create();
    container->setID("stats-container"_spr);
    container->setZOrder(2);
    container->ignoreAnchorPointForPosition(false);
    container->setAnchorPoint({1.f, 1.f});
    container->setContentSize({COLUMN_WIDTH, availableHeight});
    container->setPosition(
        {safeArea.getMaxX() - RIGHT_MARGIN, safeArea.getMaxY() - TOP_MARGIN}
    );
    container->setScale(STATS_SCALE);
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

CCNode* StatsGarageLayer::createStatItemContainer(
    Label* label, 
    CCNode* icon, 
    std::string_view id
) {
    auto container = CCNode::create();
    container->setID(fmt::format("{}-container", id));
    container->setContentSize({COLUMN_WIDTH, ITEM_HEIGHT});
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

int StatsGarageLayer::getElementsPerPage() {
    auto* fields = m_fields.self();
    float const height = fields->m_availableHeight;
    int const total = static_cast<int>(fields->m_statNodes.size());

    int const fitWithoutArrows = 
        static_cast<int>((height + ITEM_GAP) / (ITEM_HEIGHT + ITEM_GAP));

    if (total <= std::min(fitWithoutArrows, ELEMENTS_PER_PAGE)) return ELEMENTS_PER_PAGE;

    int const fitWithArrows = 
        static_cast<int>((height - 2 * ARROW_HEIGHT - ITEM_GAP) / (ITEM_HEIGHT + ITEM_GAP));

    return std::clamp(fitWithArrows, 1, ELEMENTS_PER_PAGE);
}

void StatsGarageLayer::setupNavigation() {
    auto* fields = m_fields.self();

    auto createArrowContainer = [this](bool isNext) -> Ref<CCNode> {
        auto btn = Button::createWithSpriteFrameName("GJ_arrow_02_001.png", [this, isNext](auto) {
            this->goToPage(m_fields->m_currentPage + (isNext ? 1 : -1));
        });

        if (auto* sprite = typeinfo_cast<CCSprite*>(btn->getDisplayNode())) {
            sprite->setRotation(isNext ? -90.f : 90.f);
        }

        btn->setID("arrow-button"_spr);
        btn->setScale(ARROW_SCALE);

        auto container = CCNode::create();
        container->setID(isNext ? "next-arrow-container"_spr : "prev-arrow-container"_spr);
        container->setContentSize({COLUMN_WIDTH, ARROW_HEIGHT});
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
    size_t const total = fields->m_statNodes.size();
    return total == 0 ? 0 : static_cast<int>((total - 1) / getElementsPerPage());
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

    auto oldNodes = std::move(fields->m_statNodes);
    fields->m_statNodes.clear();

    for (auto const& def : DEFAULT_STATS) {
        if (Mod::get()->getSettingValue<bool>(def.setting)) {
            auto* sprite = CCSprite::createWithSpriteFrameName(def.spriteFrame.c_str());
            int const num = GameStatsManager::sharedState()->getStat(def.statNum.c_str());
            addStatItem(fields->m_statNodes, def.id, sprite, def.scale, num);
        }
    }

    StatsManager::get()->forEachStat([this, fields](std::string_view id, StatItem& stat) {
        if (stat.provider) {
            if (auto* node = stat.provider()) {
                addStatItem(
                    fields->m_statNodes, 
                    id,
                    node, 
                    stat.nodeScale, 
                    stat.displayedNumber
                );
            }
        }
    });

    goToPage(fields->m_currentPage);

    for (auto& node : oldNodes) {
        node->cleanup();
    }

    fields->m_isRebuilding = false;
}

void StatsGarageLayer::renderCurrentPage() {
    auto* fields = m_fields.self();
    if (!fields->m_statsContainer) return;

    fields->m_statsContainer->removeAllChildrenWithCleanup(false);

    int const total = static_cast<int>(fields->m_statNodes.size());
    int const maxPage = getMaxPage();
    int const perPage = getElementsPerPage();
    bool const needsPagination = total > perPage;

    auto updateArrowState = [](CCNode* container, bool enabled) {
        if (!container) return;

        if (auto* btn = container->getChildByType<Button>()) {
            btn->setEnabled(enabled);
            btn->setOpacity(enabled ? 255 : 100);
        }
    };

    if (needsPagination) {
        updateArrowState(fields->m_prevArrow, fields->m_currentPage > 0);
        fields->m_statsContainer->addChild(fields->m_prevArrow);
    }

    size_t const start = static_cast<size_t>(fields->m_currentPage) * perPage;
    size_t const end = std::min(start + perPage, fields->m_statNodes.size());

    for (size_t i = start; i < end; ++i) {
        fields->m_statsContainer->addChild(fields->m_statNodes[i]);
    }

    if (needsPagination) {
        updateArrowState(fields->m_nextArrow, fields->m_currentPage < maxPage);
        fields->m_statsContainer->addChild(fields->m_nextArrow);
    }

    fields->m_statsContainer->updateLayout();
}
