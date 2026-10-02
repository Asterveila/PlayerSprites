#include <Geode/modify/GJGarageLayer.hpp>
#include "SpritePackListPopup.hpp"

using namespace playersprites;
using namespace geode::prelude;

class $modify(PSGarageLayer, GJGarageLayer) {

    bool init() {
        if (!GJGarageLayer::init()) return false;

		auto editorSprite = CircleButtonSprite::create(CCSprite::createWithSpriteFrameName("diffIcon_05_btn_001.png"), CircleBaseColor::Blue, CircleBaseSize::SmallAlt);
        auto editorButton = CCMenuItemSpriteExtra::create(
            editorSprite,
            this,
            menu_selector(PSGarageLayer::onOpenPackList)
        );

        auto menu = this->getChildByID("shards-menu");
        if (menu) {
            editorButton->setID("icon-workbench"_spr);
            editorButton->setPosition({-180.0f, 120.0f});
            menu->addChild(editorButton);
            menu->updateLayout();
        }

        return true;
    }

    void onOpenPackList(CCObject* sender) { SpritePackListPopup::create()->show(); }

};