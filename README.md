# Stats Display API
An API mod for **mod developers** to add their own stats to the icon kit menu.

### NOTE: This mod is intended for developers only. If you're not a mod developer, you likely won't need this mod.

## Setup
Add the mod as a dependency in your mod's `mod.json`: 

```json
"dependencies": {
    "capeling.garage-stats-menu": ">=v1.4.0"
}
```

## Usage
Include `capeling.garage-stats-menu/include/StatsDisplayAPI.h` and use `registerStatItem` in an `$execute` block, here's an example code with all the provided APIs listed:

```cpp
#include <capeling.garage-stats-menu/include/StatsDisplayAPI.h>

using namespace stats_api;

$execute {
    registerStatItem(
        // A unique ID for your stat item.
        "your-stat-item-id"_spr,
        // Lambda that determines the display node for your stat item (CCNode*).
        []() -> cocos2d::CCNode* {
            return cocos2d::CCNode::create();
        },
        // The displayed number for your stat item.
        1,
        // Optional: The node scale for your stat item. (Default: 0.5f)
        1.f
    );

    // Update the display node of your stat item.
    // The node scale parameter is optional! The default scale is 0.5f.
    updateDisplayNode("your-stat-item-id"_spr, CCNode::create(), 0.5f);

    // Unregister your stat item.
    unregisterStatItem("your-stat-item-id"_spr);

    // Get the displayed number of your stat item, returns a geode::Result<int>.
    getDisplayedNumber("your-stat-item-id"_spr).unwrapOrDefault();

    // Set the displayed number of your stat item.
    setDisplayedNumber("your-stat-item-id"_spr, 1);
}
```

## Migrating from the legacy API
Migrating is very easy; Change the header from `StatsDisplayAPI.h` to `stats_api.hpp` and simply stop hooking `GJGarageLayer::init` and instead use `registerStatItem` inside an `$execute` block.

Here's an example code that uses the old API:

```cpp
#include <capeling.garage-stats-menu/include/StatsDisplayAPI.h>

class $modify(GJGarageLayer) {
	bool init() {
		if (!GJGarageLayer::init())
			return false;

		auto statMenu = this->getChildByID("capeling.garage-stats-menu/stats-menu");

		auto myStatItem = StatsDisplayAPI::getNewItem("fire-shards"_spr, CCSprite::createWithSpriteFrameName("fireShardSmall_001.png"), GameStatsManager::sharedState()->getStat("16"), 0.8f);

		if (statMenu) {
			statMenu->addChild(myStatItem);
			statMenu->updateLayout();
		}

		return true;
	}
};
```

And here's how it can be replaced with the new API:

```cpp
#include <capeling.garage-stats-menu/include/stats_api.hpp>

using namespace stats_api;

$execute {
    registerStatItem(
        "fire-shards"_spr,
        []() -> cocos2d::CCNode* {
            CCSprite::createWithSpriteFrameName("fireShardSmall_001.png"),
        },
        GameStatsManager::sharedState()->getStat("16"),
        0.8f
    );
}
```

## Credits
- [Capeling](https://github.com/capeling): Original creator of the mod.
- [OmgRod](https://github.com/OmgRod): Previous maintainer of the mod.
- [slideglide](https://github.com/slideglide): Current maintainer of the mod.