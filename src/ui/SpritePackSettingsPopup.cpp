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

		if (!Popup::init(400.f, 320.f, "GJ_square01.png")) return false;

		this->setTitle(pack ? pack->meta.setName : "Pack Not Found");

		auto size = this->m_mainLayer->getContentSize();
		float midX = size.width / 2.f;

		auto packRow = CCMenu::create();
		packRow->setPosition({ midX, size.height - 44.f });
		this->m_mainLayer->addChild(packRow, 2);

		auto packLabel = CCLabelBMFont::create("Pack Enabled", "bigFont.fnt");
		packLabel->setScale(0.5f);
		packLabel->setAnchorPoint({ 1.f, 0.5f });
		packLabel->setPosition({ -10.f, 0.f });
		packRow->addChild(packLabel);

		auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
		auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
		m_packToggle = CCMenuItemToggler::create(offSpr, onSpr, this, menu_selector(SpritePackSettingsPopup::onTogglePack));
		m_packToggle->setScale(0.7f);
		m_packToggle->setPosition({ 10.f, 0.f });
		m_packToggle->toggle(settings::isPackEnabled(m_packId));
		packRow->addChild(m_packToggle);

		m_rowScrollLayer = ScrollLayer::create({ SettingsRowCell::WIDTH, 210.f });
		m_rowScrollLayer->setID("settings-row-scroll-layer");
		m_rowScrollLayer->ignoreAnchorPointForPosition(false);
		m_rowScrollLayer->setAnchorPoint({ 0.5f, 0.5f });
		m_rowScrollLayer->setPosition({ midX, size.height / 2.f - 25.f });
		this->m_mainLayer->addChild(m_rowScrollLayer, 1);

		auto scrollBg = CCLayerColor::create();
		scrollBg->setColor({ 0, 0, 0 });
		scrollBg->setOpacity(60);
		scrollBg->ignoreAnchorPointForPosition(false);
		scrollBg->setAnchorPoint({ 0.5f, 0.5f });
		scrollBg->setContentSize({ SettingsRowCell::WIDTH + 4.f, m_rowScrollLayer->getContentSize().height + 4.f });
		scrollBg->setPosition(m_rowScrollLayer->getPosition());
		this->m_mainLayer->addChild(scrollBg, 0);

		this->buildRows();

		return true;
	}

	void SpritePackSettingsPopup::buildRows() {
		m_rowScrollLayer->m_contentLayer->removeAllChildren();

		auto* pack = PackManager::get().findPack(m_packId);
		std::vector<CCLayer*> cells;

		if (pack) {
			std::vector<std::string> gamemodeNames;
			gamemodeNames.reserve(pack->gamemodes.size());
			for (auto const& [gm, gmEvents] : pack->gamemodes) gamemodeNames.push_back(gm);
			std::sort(gamemodeNames.begin(), gamemodeNames.end());

			for (auto const& gamemode : gamemodeNames) {
				auto const& gmEvents = pack->gamemodes.at(gamemode);

				auto* headerCell = SettingsRowCell::create(
					gamemode, settings::isGamemodeEnabled(m_packId, gamemode), false,
					cells.size() % 2 == 0, this, menu_selector(SpritePackSettingsPopup::onToggleGamemode)
				);
				headerCell->getToggle()->setUserObject("row-gamemode"_spr, CCString::create(gamemode));
				cells.push_back(headerCell);

				std::vector<std::string> eventIds;
				eventIds.reserve(gmEvents.events.size());
				for (auto const& [eventId, ev] : gmEvents.events) eventIds.push_back(eventId);
				std::sort(eventIds.begin(), eventIds.end());

				for (auto const& eventId : eventIds) {
					auto* eventCell = SettingsRowCell::create(
						eventId, settings::isEventEnabled(m_packId, gamemode, eventId), true,
						cells.size() % 2 == 0, this, menu_selector(SpritePackSettingsPopup::onToggleEvent)
					);
					eventCell->getToggle()->setUserObject("row-gamemode"_spr, CCString::create(gamemode));
					eventCell->getToggle()->setUserObject("row-event"_spr, CCString::create(eventId));
					cells.push_back(eventCell);
				}
			}
		}

		for (size_t i = 0; i < cells.size(); i++) {
			cells[i]->setPosition({ 0.f, SettingsRowCell::HEIGHT * static_cast<float>(cells.size() - 1 - i) });
			m_rowScrollLayer->m_contentLayer->addChild(cells[i]);
		}

		m_rowScrollLayer->m_contentLayer->setContentSize({
			m_rowScrollLayer->m_contentLayer->getContentSize().width,
			SettingsRowCell::HEIGHT * static_cast<float>(std::max<size_t>(cells.size(), 1))
		});
		m_rowScrollLayer->moveToTop();
	}

	void SpritePackSettingsPopup::onTogglePack(CCObject*) {
		bool newState = !settings::isPackEnabled(m_packId);
		settings::setPackEnabled(m_packId, newState);
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

}
