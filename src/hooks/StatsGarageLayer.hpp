#pragma once

#include <Geode/modify/GJGarageLayer.hpp>
#include <Geode/ui/Label.hpp>
#include <Geode/ui/Button.hpp>
#include <array>
#include <string_view>
#include <vector>

struct StatDef {
    std::string_view setting;
    std::string_view id;
    geode::ZStringView spriteFrame;
    geode::ZStringView statNum;
    float scale;
};

inline constexpr int ELEMENTS_PER_PAGE = 10;
inline constexpr float TOP_MARGIN = 6.f;
inline constexpr float RIGHT_MARGIN = 8.f;
inline constexpr float ITEM_GAP = 3.f;
inline constexpr float ARROW_SCALE = 0.5f;

inline const auto DEFAULT_STATS = std::to_array<StatDef>({
    {"stars-stat", "stars", "GJ_starsIcon_001.png", "6", 0.54f},
    {"moons-stat", "moons", "GJ_moonsIcon_001.png", "28", 0.54f},
    {"gold-coins-stat", "coins", "GJ_coinsIcon_001.png", "8", 0.51f},
    {"user-coins-stat", "user-coins", "GJ_coinsIcon2_001.png", "12", 0.51f},
    {"orbs-stat", "orbs", "currencyOrbIcon_001.png", "14", 0.54f},
    {"diamonds-stat", "diamonds", "GJ_diamondsIcon_001.png", "13", 0.6f},
    {"diamond-shards-stat", "diamond-shards", "currencyDiamondIcon_001.png", "29", 0.54f},
});

class $modify(StatsGarageLayer, GJGarageLayer) {
    struct Fields {
        geode::Ref<cocos2d::CCNode> m_statsContainer = nullptr;
        geode::Ref<cocos2d::CCNode> m_prevArrow = nullptr;
        geode::Ref<cocos2d::CCNode> m_nextArrow = nullptr;
        std::vector<geode::Ref<cocos2d::CCNode>> m_statNodes;
        geode::ListenerHandle m_statListener;
        int m_currentPage = 0;

        ~Fields() {
            m_statListener.destroy();
        }
    };

    static void onModify(auto& self);
    bool init();

private:
    static cocos2d::CCNode* createStatsContainer(const cocos2d::CCRect& safeArea);
    static cocos2d::CCNode* createStatItemContainer(geode::Label* label, cocos2d::CCNode* icon, std::string_view id);
    void addStatItem(std::vector<geode::Ref<cocos2d::CCNode>>& target, std::string_view id, geode::Ref<cocos2d::CCNode> icon, float scale, int number);

    static float getAdaptiveScale(const cocos2d::CCRect& safeArea);
    static void setArrowState(geode::Button* button, bool enabled);

    void setupNavigation();
    int getMaxPage();
    void goToPage(int page);
    void rebuildStats();
    void applyPagination();
    void renderCurrentPage();
};
