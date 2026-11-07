#include "SpritePackSettingsPopup.hpp"
#include "SettingsRowCell.hpp"
#include "SpritePackSoundsPopup.hpp"
#include "UIUtils.hpp"
#include "../data/PackManager.hpp"
#include "../data/PackSettings.hpp"

namespace playersprites {

	SpritePackSettingsPopup* SpritePackSettingsPopup::create(std::string const& packId) {
		auto ret = new SpritePackSettingsPopup();
		ret->m_packId = packId;
		if (ret->init()) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	bool SpritePackSettingsPopup::init() {
		auto* pack = PackManager::get().findPack(m_packId);

		// @geode-ignore(unknown-resource)
		if (!Popup::init(440.f, 280.f, "settingsSquare.png"_spr)) return false;

		this->setTitle(pack ? pack->meta.setName : "Pack Not Found");

		auto size = this->m_mainLayer->getContentSize();
		float midX = size.width / 2.f;
		float midY = size.height / 2.f;

		auto controlsRow = UIUtils::rowMenu(2.f, AxisAlignment::Center, AxisAlignment::Center, false, "mode-toggles-row");

		float togglerScales = 0.9f;

		auto classicBtn = UIUtils::rawVisToggler(
			settings::isModeEnabled(m_packId, false),
			this, 
			menu_selector(SpritePackSettingsPopup::onToggleClassic),
			togglerScales, 
			"classic-toggler-row",
			CCSprite::createWithSpriteFrameName("classicSmallOff.png"_spr),
			CCSprite::createWithSpriteFrameName("classicSmallOn.png"_spr)
		);
		controlsRow->addChild(classicBtn);

		auto platformerBtn = UIUtils::rawVisToggler(
			settings::isModeEnabled(m_packId, true),
			this, 
			menu_selector(SpritePackSettingsPopup::onTogglePlatformer),
			togglerScales, 
			"platformer-toggler-row",
			CCSprite::createWithSpriteFrameName("platSmallOff.png"_spr),
			CCSprite::createWithSpriteFrameName("platSmallOn.png"_spr)
		);
		controlsRow->addChild(platformerBtn);

		// SFX stuff only shows up for packs that actually come with a sounds.json file
		bool hasSounds = pack && !pack->sounds.empty();
		if (hasSounds) {
			auto sfxBtn = UIUtils::rawVisToggler(
				settings::isSfxEnabled(m_packId),
				this, 
				menu_selector(SpritePackSettingsPopup::onToggleSfx), 
				togglerScales,
				"sfx-toggler-row",
				CCSprite::createWithSpriteFrameName("sfxSmallOff.png"_spr),
				CCSprite::createWithSpriteFrameName("sfxSmallOn.png"_spr)
			);
			controlsRow->addChild(sfxBtn);
		}

		controlsRow->updateLayout();
		controlsRow->setPosition({ midX, size.height - 47.f });
		// controlsRow->setScale(0.85f);
		this->m_mainLayer->addChild(controlsRow, 2);

		bool hasTaunts = false;
		if (pack) {
			for (auto const& [gm, gmEvents] : pack->gamemodes) {
				if (!gmEvents.taunts.empty()) { hasTaunts = true; break; }
			}
		}
		if (hasTaunts) {
			auto viewColumn = UIUtils::columnMenu(2.f, AxisAlignment::Center, AxisAlignment::Center, false, "view-toggles-column", 0.f, 1.f);

			float btnScales = 0.4f;
			float btnHeights = 25.f;
			auto eventsOff = ButtonSprite::create("Events", 0.f, false, "bigFont.fnt", "inactiveButton.png"_spr, btnHeights, btnScales);
			auto eventsOn = ButtonSprite::create("Events", 0.f, false, "bigFont.fnt", "activeButton.png"_spr, btnHeights, btnScales);
			auto tauntsOff = ButtonSprite::create("Taunts", 0.f, false, "bigFont.fnt", "inactiveButton.png"_spr, btnHeights, btnScales);
			auto tauntsOn = ButtonSprite::create("Taunts", 0.f, false, "bigFont.fnt", "activeButton.png"_spr, btnHeights, btnScales);

			auto eventsBtn = UIUtils::rawVisToggler(
				true, this, menu_selector(SpritePackSettingsPopup::onShowEvents),
				togglerScales, "events-toggler-row", eventsOff, eventsOn
			);
			m_eventsToggler = eventsBtn;
			viewColumn->addChild(m_eventsToggler);

			auto tauntsBtn = UIUtils::rawVisToggler(
				false, this, menu_selector(SpritePackSettingsPopup::onShowTaunts),
				togglerScales, "taunts-toggler-row", tauntsOff, tauntsOn
			);
			m_tauntsToggler = tauntsBtn;
			viewColumn->addChild(m_tauntsToggler);

			viewColumn->setPosition({ 25.f, size.height - 10.f });
			viewColumn->updateLayout();
			this->m_mainLayer->addChild(viewColumn, 2);
		}

		if (pack) {
			auto exportArrow = CCSprite::createWithSpriteFrameName("exportBtn.png"_spr);
			auto exportSpr = CircleButtonSprite::create(exportArrow, CircleBaseColor::Pink, CircleBaseSize::Small);
			// exportSpr->setScale(1.f);
			auto exportBtn = CCMenuItemSpriteExtra::create(exportSpr, this, menu_selector(SpritePackSettingsPopup::onExport));
			exportBtn->setID("export-pack-btn");
			exportBtn->setPosition({ size.width - 30.f, size.height - 32.f });

			exportArrow->setPositionY(exportArrow->getPositionY() + 2.f);

			auto exportMenu = CCMenu::create();
			exportMenu->setID("export-menu");
			exportMenu->setPosition({ 0.f, 0.f });
			exportMenu->addChild(exportBtn);

			auto trashSpr = CircleButtonSprite::createWithSpriteFrameName("trashcan.png"_spr, 0.8f, CircleBaseColor::Red, CircleBaseSize::Small);
			// trashSpr->setScale(1.1f);
			auto trashBtn = CCMenuItemSpriteExtra::create(trashSpr, this, menu_selector(SpritePackSettingsPopup::onDelete));
			trashBtn->setID("delete-pack-btn");
			trashBtn->setPosition({ size.width - 30.f - 43.f, size.height - 32.f });
			exportMenu->addChild(trashBtn);
			
			this->m_mainLayer->addChild(exportMenu, 2);
		}

		m_rowScrollLayer = ScrollLayer::create({ SettingsRowCell::WIDTH, 200.f });
		m_rowScrollLayer->setID("settings-row-scroll-layer");
		m_rowScrollLayer->ignoreAnchorPointForPosition(false);
		m_rowScrollLayer->setAnchorPoint({ 0.5f, 0.5f });
		m_rowScrollLayer->setPosition({ midX, midY - 25.f });
		this->m_mainLayer->addChild(m_rowScrollLayer, 1);

		auto scrollBg = CCLayerColor::create();
		scrollBg->setColor({ 0, 0, 0 });
		scrollBg->setOpacity(60);
		scrollBg->ignoreAnchorPointForPosition(false);
		scrollBg->setAnchorPoint({ 0.5f, 0.5f });
		scrollBg->setContentSize({ SettingsRowCell::WIDTH + 4.f, m_rowScrollLayer->getContentSize().height + 4.f });
		scrollBg->setPosition(m_rowScrollLayer->getPosition());
		this->m_mainLayer->addChild(scrollBg, 0);

		auto controlsMenu = CCMenu::create();
		controlsMenu->setPosition({ 0.f, 0.f });

		if (hasSounds) {
			auto sfxLabel = CCLabelBMFont::create("SFX", "bigFont.fnt");
			sfxLabel->setScale(0.5f);
			auto sfxSpr = CircleButtonSprite::create(sfxLabel, CircleBaseColor::Blue, CircleBaseSize::Small);
			sfxSpr->setScale(0.8f);
			auto sfxBtn = CCMenuItemSpriteExtra::create(sfxSpr, this, menu_selector(SpritePackSettingsPopup::onOpenSounds));
			sfxBtn->setID("open-sounds-btn");
			sfxBtn->setPosition({ size.width, 0.f });
			controlsMenu->addChild(sfxBtn);
		}

		this->m_mainLayer->addChild(controlsMenu, 2);

		this->buildRows();

		return true;
	}

	void SpritePackSettingsPopup::buildRows() {
		m_rowScrollLayer->m_contentLayer->removeAllChildren();

		auto* pack = PackManager::get().findPack(m_packId);
		std::vector<SettingsRowCell*> cells;

		if (pack) {
			std::vector<std::string> gamemodeNames;
			gamemodeNames.reserve(pack->gamemodes.size());
			for (auto const& [gm, gmEvents] : pack->gamemodes) gamemodeNames.push_back(gm);
			std::sort(gamemodeNames.begin(), gamemodeNames.end());

			for (auto const& gamemode : gamemodeNames) {
				auto const& gmEvents = pack->gamemodes.at(gamemode);

				auto* headerCell = SettingsRowCell::createHeader(
					gamemode, settings::isGamemodeEnabled(m_packId, gamemode),
					cells.size() % 2 == 0, this, menu_selector(SpritePackSettingsPopup::onToggleGamemode)
				);
				headerCell->getToggle()->setUserObject("row-gamemode"_spr, CCString::create(gamemode));
				cells.push_back(headerCell);

				std::vector<std::string> eventIds;
				eventIds.reserve(gmEvents.events.size());
				for (auto const& [eventId, ev] : gmEvents.events) eventIds.push_back(eventId);
				std::sort(eventIds.begin(), eventIds.end());

				for (auto const& eventId : eventIds) {
					auto const& anim = gmEvents.events.at(eventId);

					auto* eventCell = SettingsRowCell::createEvent(
						*pack, anim, settings::isEventEnabled(m_packId, gamemode, eventId),
						cells.size() % 2 == 0, this, menu_selector(SpritePackSettingsPopup::onToggleEvent)
					);
					eventCell->getToggle()->setUserObject("row-gamemode"_spr, CCString::create(gamemode));
					eventCell->getToggle()->setUserObject("row-event"_spr, CCString::create(eventId));
					cells.push_back(eventCell);
				}
			}
		}

		/*
		float totalHeight = 0.f;
		for (auto* cell : cells) totalHeight += cell->getContentSize().height;

		float y = totalHeight;
		for (auto* cell : cells) {
			y -= cell->getContentSize().height;
			cell->setPosition({ 0.f, y });
			m_rowScrollLayer->m_contentLayer->addChild(cell);
		}

		m_rowScrollLayer->m_contentLayer->setContentSize({
			m_rowScrollLayer->m_contentLayer->getContentSize().width,
			std::max(totalHeight, SettingsRowCell::HEADER_HEIGHT)
		});
		m_rowScrollLayer->moveToTop();
		*/
		this->layoutCells(m_rowScrollLayer, cells);
	}

	void SpritePackSettingsPopup::onToggleClassic(CCObject*) {
		bool newState = !settings::isModeEnabled(m_packId, false);
		settings::setModeEnabled(m_packId, false, newState);
	}

	void SpritePackSettingsPopup::onTogglePlatformer(CCObject*) {
		bool newState = !settings::isModeEnabled(m_packId, true);
		settings::setModeEnabled(m_packId, true, newState);
	}

	void SpritePackSettingsPopup::onToggleSfx(CCObject*) {
		bool newState = !settings::isSfxEnabled(m_packId);
		settings::setSfxEnabled(m_packId, newState);
	}

	void SpritePackSettingsPopup::onOpenSounds(CCObject*) {
		SpritePackSoundsPopup::create(m_packId)->show();
	}

	void SpritePackSettingsPopup::onToggleGamemode(CCObject* sender) {
		auto* btn = static_cast<CCMenuItemToggler*>(sender);
		auto* gmObj = static_cast<CCString*>(btn->getUserObject("row-gamemode"_spr));
		if (!gmObj) return;
		std::string gamemode = gmObj->getCString();

		bool newState = !settings::isGamemodeEnabled(m_packId, gamemode);
		settings::setGamemodeEnabled(m_packId, gamemode, newState);
	}

	void SpritePackSettingsPopup::onToggleEvent(CCObject* sender) {
		auto* btn = static_cast<CCMenuItemToggler*>(sender);
		auto* gmObj = static_cast<CCString*>(btn->getUserObject("row-gamemode"_spr));
		auto* evObj = static_cast<CCString*>(btn->getUserObject("row-event"_spr));
		if (!gmObj || !evObj) return;
		std::string gamemode = gmObj->getCString();
		std::string eventId = evObj->getCString();

		bool newState = !settings::isEventEnabled(m_packId, gamemode, eventId);
		settings::setEventEnabled(m_packId, gamemode, eventId, newState);
	}

	void SpritePackSettingsPopup::layoutCells(ScrollLayer* layer, std::vector<SettingsRowCell*> const& cells) {
		float totalHeight = 0.f;
		for (auto* cell : cells) totalHeight += cell->getContentSize().height;

		float y = totalHeight;
		for (auto* cell : cells) {
			y -= cell->getContentSize().height;
			cell->setPosition({ 0.f, y });
			layer->m_contentLayer->addChild(cell);
		}

		layer->m_contentLayer->setContentSize({
			layer->m_contentLayer->getContentSize().width,
			std::max(totalHeight, SettingsRowCell::HEADER_HEIGHT)
		});
		layer->moveToTop();
	}

	void SpritePackSettingsPopup::buildTauntRows() {
		m_rowScrollLayer->m_contentLayer->removeAllChildren();

		auto* pack = PackManager::get().findPack(m_packId);
		std::vector<SettingsRowCell*> cells;

		if (pack) {
			std::vector<std::string> gamemodeNames;
			for (auto const& [gm, gmEvents] : pack->gamemodes) {
				if (!gmEvents.taunts.empty()) gamemodeNames.push_back(gm);
			}
			std::sort(gamemodeNames.begin(), gamemodeNames.end());

			for (auto const& gamemode : gamemodeNames) {
				auto const& gm = pack->gamemodes.at(gamemode);

				auto* headerCell = SettingsRowCell::createHeader(
					gamemode, settings::isGamemodeEnabled(m_packId, gamemode),
					cells.size() % 2 == 0, this, menu_selector(SpritePackSettingsPopup::onToggleGamemode), "Taunts"
				);
				headerCell->getToggle()->setUserObject("row-gamemode"_spr, CCString::create(gamemode));
				cells.push_back(headerCell);

				for (auto const& taunt : gm.taunts) {
					auto* tauntCell = SettingsRowCell::createTaunt(
						*pack, taunt, gm.tauntsOrdered, settings::isTauntEnabled(m_packId, gamemode, taunt.anim.id),
						cells.size() % 2 == 0, this, menu_selector(SpritePackSettingsPopup::onToggleTaunt)
					);
					tauntCell->getToggle()->setUserObject("row-gamemode"_spr, CCString::create(gamemode));
					tauntCell->getToggle()->setUserObject("row-taunt"_spr, CCString::create(taunt.anim.id));
					cells.push_back(tauntCell);
				}
			}
		}

		this->layoutCells(m_rowScrollLayer, cells);
	}

	void SpritePackSettingsPopup::setView(bool showTaunts) {
		auto* clicked = showTaunts ? m_tauntsToggler : m_eventsToggler;
		auto* other = showTaunts ? m_eventsToggler : m_tauntsToggler;

		if (showTaunts == m_showingTaunts) {
			if (clicked) clicked->toggle(false);
			return;
		}

		m_showingTaunts = showTaunts;
		if (other) other->toggle(false);

		if (showTaunts) this->buildTauntRows();
		else this->buildRows();
	}

	void SpritePackSettingsPopup::onShowEvents(CCObject*) { this->setView(false); }
	void SpritePackSettingsPopup::onShowTaunts(CCObject*) { this->setView(true); }

	void SpritePackSettingsPopup::onToggleTaunt(CCObject* sender) {
		auto* btn = static_cast<CCMenuItemToggler*>(sender);
		auto* gmObj = static_cast<CCString*>(btn->getUserObject("row-gamemode"_spr));
		auto* tauntObj = static_cast<CCString*>(btn->getUserObject("row-taunt"_spr));
		if (!gmObj || !tauntObj) return;
		std::string gamemode = gmObj->getCString();
		std::string tauntId = tauntObj->getCString();

		bool newState = !settings::isTauntEnabled(m_packId, gamemode, tauntId);
		settings::setTauntEnabled(m_packId, gamemode, tauntId, newState);
	}

	void SpritePackSettingsPopup::onDelete(CCObject*) {
		auto* pack = PackManager::get().findPack(m_packId);
		if (!pack) return;

		std::string packId = m_packId;
		std::string packName = pack->meta.setName;

		geode::createQuickPopup(
			"Delete Pack",
			fmt::format(
				"Are you sure to delete <cy>{}</c>? <cr>This can NOT be undone!!</c>\n\n"
				"The pack will be deleted off of your pack registry and its folder (along with <cj>all its contents</c>) will be <cr>permanently deleted!!</c>",
				packName
			),
			"Cancel", "Yes, delete",
			[this, packId, packName](FLAlertLayer*, bool yes) {
				if (!yes) return;

				auto res = PackManager::get().deletePack(packId);

				if (res.isErr() && PackManager::get().registry().contains(packId)) {
					FLAlertLayer::create("Couldn't delete pack", res.unwrapErr(), "OK")->show();
					return;
				}

				if (m_onPackDeleted) m_onPackDeleted();

				if (res.isErr()) {
					FLAlertLayer::create("Pack removed", res.unwrapErr(), "OK")->show();
				} else {
					Notification::create(fmt::format("{} deleted successfully!", packName), NotificationIcon::Success)->show();
				}

				this->onClose(nullptr);
			}
		);
	}

	/*
	void SpritePackSettingsPopup::onOpenFolder(CCObject*) {
		auto* pack = PackManager::get().findPack(m_packId);
		if (!pack) return;
		geode::utils::file::openFolder(pack->rootPath);
	}
	*/

	void SpritePackSettingsPopup::onExport(CCObject*) {
		auto res = transfer::exportPack(m_packId);
		if (res.isErr()) {
			FLAlertLayer::create("Export failed...", res.unwrapErr(), "aw...")->show();
			return;
		}

		auto zipPath = std::move(res).unwrap();
		FLAlertLayer::create(
			"Export Successful!", 
			fmt::format("Successfully exported pack to <cy>{}</c>! \n\nIf this isn't your preferred folder for exports, please change it in the mod's config.", geode::utils::string::pathToString(zipPath)),
			"OK"
		)->show();

		/*
		Notification::create(
			fmt::format("Exported {}", geode::utils::string::pathToString(zipPath.filename())),
			NotificationIcon::Success
		)->show();
		*/

		#ifdef GEODE_IS_DESKTOP
			file::openFolder(zipPath.parent_path()); // this would prolly get annoying on android chat im #ngl
		#endif
	}

}
