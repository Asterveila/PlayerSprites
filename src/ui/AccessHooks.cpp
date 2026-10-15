#include <Geode/modify/GJGarageLayer.hpp>
#include "SpritePackListPopup.hpp"
#include "../data/PackManager.hpp"

using namespace playersprites;
using namespace geode::prelude;

$on_mod(Loaded) {
    PackManager::get().reload();
};

class $modify(PSGarageLayer, GJGarageLayer) {

    bool init() {
        if (!GJGarageLayer::init()) return false;

		// @geode-ignore(unknown-resource)
		//auto editorSprite = CircleButtonSprite::create(CCSprite::createWithSpriteFrameName("geode.loader/grid-view.png"), CircleBaseColor::Blue, CircleBaseSize::SmallAlt);
        
        auto openMenuBase = CCSprite::createWithSpriteFrameName("bigSquareBtnBlue.png"_spr);
        auto openMenuSpr = CCSprite::createWithSpriteFrameName("menuBtnSprite.png"_spr);

        openMenuBase->addChild(openMenuSpr, 2);
        openMenuSpr->setPosition({openMenuBase->getContentWidth() / 2.f - 0.5f, openMenuBase->getContentHeight() / 2.f + 1.25f});
        openMenuSpr->setScale(1.1f);

        auto editorButton = CCMenuItemSpriteExtra::create(
            openMenuBase,
            this,
            menu_selector(PSGarageLayer::onOpenPackList)
        );

        auto menu = this->getChildByID("shards-menu");
        if (menu) {
            editorButton->setID("sprite-packs"_spr);
            editorButton->setPosition({-180.0f, 120.0f});
            menu->addChild(editorButton);
            menu->updateLayout();
        }

        return true;
    }

    void onOpenPackList(CCObject* sender) {

        if (!Mod::get()->getSavedValue("installed-builtin", false)) {
            MDPopup::create(
                "Install Built-in Packs",
                "Hey there! It looks like this is your first time using <cy>PlayerSprites</c>, welcome! \n\nThis mod is completely driven by **'Sprite Packs'**, which you have none of right now. \n\nDon't worry, though! The mod comes with a few of them <cj>built-in</c> so you can immediately get to experience what the mod can do! \n\n## <cs>Click on the button below to install them!</c> \nOnce they're installed, we'll take you directly to the mod's popup where you can properly configure everything about all sprite packs you have!",
                "Install", nullptr,
                [](bool) {
                    auto res = transfer::installBuiltinPacks();
                    if (res.isErr()) {
                        FLAlertLayer::create("Install failed...", res.unwrapErr(), "OK")->show();
                    } else {
                        Notification::create(fmt::format("Installed {} packs!", res.unwrap()), NotificationIcon::Success)->show();
                        Mod::get()->setSavedValue("installed-builtin", true);
                    }
                    SpritePackListPopup::create()->show();
                    MDPopup::create(
                        "PlayerSprites Menu",
                        "Welcome to the <cy>PlayerSprites menu</c>! \n\nFrom here, you can <cj>enable/disable</c> entire sprite packs to your liking, open the Packs folder, and <cj>import new packs</c> with ease! \n\nClick on the <cg>gear icon</c> on any Sprite Pack to configure it even further, <cf>enable/disable individual animations</c>, <cp>control gamemodes</c>, <cj>SFX</c>, and <co>level types</c>! \n\nAny pack you import is <cr>**disabled by default**</c>, so PLEASE **enable the full pack before going into a level.** If the pack remains on its default state (off), you won't see any effect in gameplay. \n<cg>Make sure your packs are enabled before reporting an issue or deeming the pack defective.</c>", 
                        "OK"
                    )->show();
                }
            )->show();
            Mod::get()->setSavedValue<int>("builtin-version", 1);
        } else {
            SpritePackListPopup::create()->show();
        }
        
    }

};