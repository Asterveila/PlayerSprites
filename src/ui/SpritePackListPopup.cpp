#include "SpritePackListPopup.hpp"
#include "SpritePackCell.hpp"
#include "SpritePackSettingsPopup.hpp"
#include "../data/PackManager.hpp"

namespace playersprites {

	SpritePackListPopup* SpritePackListPopup::create() {
		auto ret = new SpritePackListPopup();
		if (ret->init()) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	bool SpritePackListPopup::init() {
		if (!Popup::init(400.f, 280.f, "GJ_square01.png")) return false;

		this->setTitle("Sprite Packs");

		auto size = this->m_mainLayer->getContentSize();
		float midX = size.width / 2.f;

		m_packScrollLayer = ScrollLayer::create({ SpritePackCell::WIDTH, 190.f });
		m_packScrollLayer->setID("sprite-pack-scroll-layer");
		m_packScrollLayer->ignoreAnchorPointForPosition(false);
		m_packScrollLayer->setAnchorPoint({ 0.5f, 0.5f });
		m_packScrollLayer->setPosition({ midX, size.height / 2.f + 5.f });
		this->m_mainLayer->addChild(m_packScrollLayer, 1);

		auto scrollBg = CCLayerColor::create();
		scrollBg->setID("sprite-pack-scroll-bg");
		scrollBg->setColor({ 0, 0, 0 });
		scrollBg->setOpacity(60);
		scrollBg->ignoreAnchorPointForPosition(false);
		scrollBg->setAnchorPoint({ 0.5f, 0.5f });
		scrollBg->setContentSize({ SpritePackCell::WIDTH + 4.f, m_packScrollLayer->getContentSize().height + 4.f });
		scrollBg->setPosition(m_packScrollLayer->getPosition());
		this->m_mainLayer->addChild(scrollBg, 0);

		auto reloadSprite = ButtonSprite::create("Reload");
		reloadSprite->setScale(0.7f);
		auto* reloadBtn = CCMenuItemSpriteExtra::create(
			reloadSprite, this, menu_selector(SpritePackListPopup::onReload)
		);
		reloadBtn->setID("reload-btn");

		auto reloadMenu = CCMenu::create();
		reloadMenu->addChild(reloadBtn);
		reloadMenu->setPosition({ midX, 22.f });
		this->m_mainLayer->addChild(reloadMenu, 2);

		this->buildList();

		return true;
	}

	void SpritePackListPopup::buildList() {
		m_packScrollLayer->m_contentLayer->removeAllChildren();

		auto& mgr = PackManager::get();
		auto const& loaded = mgr.getLoadedPacks();
		auto const& errors = mgr.getLoadErrors();

		std::vector<CCLayer*> cells;
		cells.reserve(loaded.size() + errors.size());

		for (auto const& pack : loaded) {
			cells.push_back(SpritePackCell::createForPack(
				pack, cells.size() % 2 == 0, this, menu_selector(SpritePackListPopup::onPackSettings)
			));
		}
		for (auto const& [id, error] : errors) {
			cells.push_back(SpritePackCell::createForError(id, error, cells.size() % 2 == 0));
		}

		for (size_t i = 0; i < cells.size(); i++) {
			cells[i]->setPosition({ 0.f, SpritePackCell::HEIGHT * static_cast<float>(cells.size() - 1 - i) });
			m_packScrollLayer->m_contentLayer->addChild(cells[i]);
		}

		m_packScrollLayer->m_contentLayer->setContentSize({
			m_packScrollLayer->m_contentLayer->getContentSize().width,
			SpritePackCell::HEIGHT * static_cast<float>(std::max<size_t>(cells.size(), 1))
		});
		m_packScrollLayer->moveToTop();
	}

	void SpritePackListPopup::onReload(CCObject*) {
		PackManager::get().reload();
		this->buildList();
	}

	void SpritePackListPopup::onPackSettings(CCObject* sender) {
		auto* btn = static_cast<CCMenuItemSpriteExtra*>(sender);
		auto* idObj = static_cast<CCString*>(btn->getUserObject("pack-id"_spr));
		if (!idObj) return;
		std::string id = idObj->getCString();

		SpritePackSettingsPopup::create(id)->show();
	}

}
