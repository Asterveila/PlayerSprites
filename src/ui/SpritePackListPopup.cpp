#include "SpritePackListPopup.hpp"
#include "SpritePackCell.hpp"
#include "SpritePackSettingsPopup.hpp"
#include "../data/PackManager.hpp"
#include "../data/PackSettings.hpp"

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
		if (!Popup::init(460.f, 280.f, "hueShiftedSquare.png"_spr)) return false;

		this->setTitle("Sprite Packs");

		auto size = this->m_mainLayer->getContentSize();
		float midX = size.width / 2.f;

		m_packScrollLayer = ScrollLayer::create({ SpritePackCell::WIDTH, 190.f });
		m_packScrollLayer->setID("sprite-pack-scroll-layer");
		m_packScrollLayer->ignoreAnchorPointForPosition(false);
		m_packScrollLayer->setAnchorPoint({ 0.5f, 0.5f });
		m_packScrollLayer->setPosition({ midX, size.height / 2.f + 7.f });
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

		auto controlsMenu = CCMenu::create();
		controlsMenu->setContentSize(size);
		controlsMenu->ignoreAnchorPointForPosition(false);
		controlsMenu->setAnchorPoint({ 0, 0 });
		controlsMenu->setPosition({ 0, 0 });
		controlsMenu->setID("controls-menu");
		this->m_mainLayer->addChild(controlsMenu);

		auto reloadSprite = ButtonSprite::create("Reload");
		reloadSprite->setScale(0.7f);
		auto* reloadBtn = CCMenuItemSpriteExtra::create(
			reloadSprite, this, menu_selector(SpritePackListPopup::onReload)
		);
		reloadBtn->setID("reload-btn");
		reloadBtn->setPosition({ midX, 22.f });

		controlsMenu->addChild(reloadBtn);

		auto settingsSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
		settingsSpr->setScale(0.75f);
		auto settingsBtn = CCMenuItemSpriteExtra::create(settingsSpr, this, menu_selector(SpritePackListPopup::onModSettings));
		settingsBtn->setID("mod-settings-btn");
		settingsBtn->setPosition({ 35.f, 28.f });

		controlsMenu->addChild(settingsBtn);
		
		auto importArrow = CCSprite::createWithSpriteFrameName("importBtn.png"_spr);
		auto importSpr = CircleButtonSprite::create(importArrow, CircleBaseColor::Green, CircleBaseSize::MediumAlt);
		importArrow->setScale(0.85f);
		importSpr->setScale(0.75f);
		importArrow->setPositionY(importArrow->getPositionY() + 2.f);
		auto importBtn = CCMenuItemSpriteExtra::create(importSpr, this, menu_selector(SpritePackListPopup::onImport));
		importBtn->setID("import-pack-btn");
		importBtn->setPosition({ settingsBtn->getContentWidth() + 40.f, 28.f });

		controlsMenu->addChild(importBtn);

		auto folderSpr = CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png");
		folderSpr->setScale(1.1f);
		auto folderBtn = CCMenuItemSpriteExtra::create(folderSpr, this, menu_selector(SpritePackListPopup::onOpenFolder));
		folderBtn->setID("open-pack-folder-btn");
		folderBtn->setPosition({ size.width - 35.f, 25.f });

		controlsMenu->addChild(folderBtn);

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
				pack, cells.size() % 2 == 0, this,
				menu_selector(SpritePackListPopup::onTogglePack),
				menu_selector(SpritePackListPopup::onPackInfo),
				menu_selector(SpritePackListPopup::onPackSettings)
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
		Notification::create(fmt::format("Reloaded {} packs!", PackManager::get().getLoadedPacks().size()), NotificationIcon::Success, 0.5f)->show();
	}

	void SpritePackListPopup::onImport(CCObject*) {
		m_importListener.spawn(file::pick(file::PickMode::OpenFile, {
			.filters = {{
				.description = "Sprite Packs",
				.files = { "*.zip" }
			}}
		}), [this](Result<std::optional<std::filesystem::path>> res) {
			if (res.isErr()) {
				FLAlertLayer::create("Import failed!", fmt::format("couldn't open the file picker: {}", res.unwrapErr()), "OK")->show();
				return;
			}

			auto path = std::move(res).unwrap();
			if (!path.has_value()) return;

			auto importRes = transfer::importPack(path.value());
			if (importRes.isErr()) {
				FLAlertLayer::create("Import failed...", importRes.unwrapErr(), "aw...")->show();
				return;
			}

			Notification::create(fmt::format("Imported '{}'!", importRes.unwrap()), NotificationIcon::Success)->show();
			this->buildList();
		});
	}

	void SpritePackListPopup::onTogglePack(CCObject* sender) {
		auto* btn = static_cast<CCMenuItemToggler*>(sender);
		auto* idObj = static_cast<CCString*>(btn->getUserObject("pack-id"_spr));
		if (!idObj) return;
		std::string id = idObj->getCString();

		bool newState = !settings::isPackEnabled(id);
		settings::setPackEnabled(id, newState);
	}

	void SpritePackListPopup::onPackInfo(CCObject* sender) {
		auto* btn = static_cast<CCMenuItemSpriteExtra*>(sender);
		auto* idObj = static_cast<CCString*>(btn->getUserObject("pack-id"_spr));
		if (!idObj) return;

		auto* pack = PackManager::get().findPack(idObj->getCString());
		if (!pack) return;

		std::string description = pack->meta.description.empty() ? "No Description Provided" : pack->meta.description;
		MDPopup::create(pack->meta.setName.c_str(), description, "OK")->show();
	}

	void SpritePackListPopup::onPackSettings(CCObject* sender) {
		auto* btn = static_cast<CCMenuItemSpriteExtra*>(sender);
		auto* idObj = static_cast<CCString*>(btn->getUserObject("pack-id"_spr));
		if (!idObj) return;
		std::string id = idObj->getCString();

		SpritePackSettingsPopup::create(id)->show();
	}

	void SpritePackListPopup::onOpenFolder(CCObject *sender) {
		utils::file::openFolder(Mod::get()->getConfigDir() / "packs");
	}

	void SpritePackListPopup::onModSettings(CCObject *sender) {
		geode::openSettingsPopup(Mod::get());
	}

}
