#include "SpritePackSettingsPopup.hpp"
#include "SettingsRowCell.hpp"
#include "../data/PackManager.hpp"
#include "../data/PackSettings.hpp"
#include <algorithm>

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

		if (!Popup::init(440.f, 280.f, "geode.loader/GE_square03.png")) return false;

		this->setTitle(pack ? pack->meta.setName : "Pack Not Found");

		auto size = this->m_mainLayer->getContentSize();
		float midX = size.width / 2.f;
		float midY = size.height / 2.f;

		bool packEnabled = settings::isPackEnabled(m_packId);
		auto statusLabel = CCLabelBMFont::create(packEnabled ? "Pack is Enabled" : "Pack is Disabled", "chatFont.fnt");
		statusLabel->setScale(0.6f);
		statusLabel->setColor(packEnabled ? ccColor3B{ 100, 255, 100 } : ccColor3B{ 255, 90, 90 });
		statusLabel->setPosition({ midX, size.height - 40.f });
		statusLabel->setID("pack-status-label");
		this->m_mainLayer->addChild(statusLabel, 2);

		m_rowScrollLayer = ScrollLayer::create({ SettingsRowCell::WIDTH, 210.f });
		m_rowScrollLayer->setID("settings-row-scroll-layer");
		m_rowScrollLayer->ignoreAnchorPointForPosition(false);
		m_rowScrollLayer->setAnchorPoint({ 0.5f, 0.5f });
		m_rowScrollLayer->setPosition({ midX, midY - 20.f });
		this->m_mainLayer->addChild(m_rowScrollLayer, 1);

		auto scrollBg = CCLayerColor::create();
		scrollBg->setColor({ 0, 0, 0 });
		scrollBg->setOpacity(60);
		scrollBg->ignoreAnchorPointForPosition(false);
		scrollBg->setAnchorPoint({ 0.5f, 0.5f });
		scrollBg->setContentSize({ SettingsRowCell::WIDTH + 4.f, m_rowScrollLayer->getContentSize().height + 4.f });
		scrollBg->setPosition(m_rowScrollLayer->getPosition());
		this->m_mainLayer->addChild(scrollBg, 0);

		auto folderSpr = CircleButtonSprite::create(CCSprite::createWithSpriteFrameName("folderIcon_001.png"), CircleBaseColor::Green, CircleBaseSize::Small);
		folderSpr->setScale(0.8f);
		auto folderBtn = CCMenuItemSpriteExtra::create(folderSpr, this, menu_selector(SpritePackSettingsPopup::onOpenFolder));
		folderBtn->setID("open-pack-folder-btn");

		auto folderMenu = CCMenu::create();
		folderMenu->setPosition({ 0.f, 0.f });
		folderMenu->addChild(folderBtn);
		this->m_mainLayer->addChild(folderMenu, 2);

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

	void SpritePackSettingsPopup::onOpenFolder(CCObject*) {
		auto* pack = PackManager::get().findPack(m_packId);
		if (!pack) return;
		geode::utils::file::openFolder(pack->rootPath);
	}

}
