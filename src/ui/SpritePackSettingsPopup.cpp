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
		if (!Popup::init(440.f, 280.f, "geode.loader/GE_square03.png")) return false;

		this->setTitle(pack ? pack->meta.setName : "Pack Not Found");

		auto size = this->m_mainLayer->getContentSize();
		float midX = size.width / 2.f;
		float midY = size.height / 2.f;

		auto modeRow = UIUtils::row(8.f, AxisAlignment::Center, AxisAlignment::Center, false, "mode-toggles-row");

		auto classicRow = UIUtils::togglerRow(
			"Enable Classic", settings::isModeEnabled(m_packId, false),
			this, menu_selector(SpritePackSettingsPopup::onToggleClassic),
			145.f, 0.35f, 0.6f, "classic-toggler-row"
		);
		modeRow->addChild(classicRow.container);

		auto platformerRow = UIUtils::togglerRow(
			"Enable Platformer", settings::isModeEnabled(m_packId, true),
			this, menu_selector(SpritePackSettingsPopup::onTogglePlatformer),
			170.f, 0.35f, 0.6f, "platformer-toggler-row"
		);
		modeRow->addChild(platformerRow.container);

		// SFX stuff only shows up for packs that actually come with a sounds.json file
		bool hasSounds = pack && !pack->sounds.empty();
		if (hasSounds) {
			auto sfxRow = UIUtils::togglerRow(
				"Enable SFX", settings::isSfxEnabled(m_packId),
				this, menu_selector(SpritePackSettingsPopup::onToggleSfx),
				110.f, 0.35f, 0.6f, "sfx-toggler-row"
			);
			modeRow->addChild(sfxRow.container);
		}

		modeRow->updateLayout();
		modeRow->setPosition({ midX, size.height - 45.f });
		modeRow->setScale(0.85f);
		this->m_mainLayer->addChild(modeRow, 2);

		if (pack) {
			auto exportArrow = CCSprite::create("exportBtn.png"_spr);
			auto exportSpr = CircleButtonSprite::create(exportArrow, CircleBaseColor::Pink, CircleBaseSize::Small);
			exportSpr->setScale(1.1f);
			auto exportBtn = CCMenuItemSpriteExtra::create(exportSpr, this, menu_selector(SpritePackSettingsPopup::onExport));
			exportBtn->setID("export-pack-btn");
			exportBtn->setPosition({ size.width - 30.f, size.height - 32.f });

			exportArrow->setPositionY(exportArrow->getPositionY() + 2.f);

			auto exportMenu = CCMenu::create();
			exportMenu->setPosition({ 0.f, 0.f });
			exportMenu->addChild(exportBtn);
			exportMenu->setID("export-menu");
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
			fmt::format("Successfully exported pack to {}! \n\nIf this isn't your preferred folder for exports, please change it in the mod's config.", geode::utils::string::pathToString(zipPath.filename())),
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
