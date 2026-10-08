#include "SpritePackSoundsPopup.hpp"
#include "SoundRowCell.hpp"
#include "../core/SoundPlayer.hpp"
#include "../data/PackManager.hpp"
#include "../data/PackSettings.hpp"
#include <algorithm>

namespace playersprites {

	namespace {

		std::string describeEntry(SoundEntry const& entry) {
			std::string text;

			bool shorthand = entry.triggerOn.size() == 1 && entry.triggerOn[0] == entry.id;
			if (shorthand) {
				for (size_t i = 0; i < entry.files.size(); ++i) {
					if (i > 0) text += ", ";
					text += geode::utils::string::pathToString(entry.files[i].filename());
				}
			} else {
				text = "Triggered by: ";
				for (size_t i = 0; i < entry.triggerOn.size(); ++i) {
					if (i > 0) text += ", ";
					text += entry.triggerOn[i];
				}
				text += fmt::format(" | {} sound{}", entry.files.size(), entry.files.size() == 1 ? "" : "s");
			}

			if (entry.ordered) text = "In order | " + text;
			return text;
		}

	}

	SpritePackSoundsPopup* SpritePackSoundsPopup::create(std::string const& packId) {
		auto ret = new SpritePackSoundsPopup();
		ret->m_packId = packId;
		if (ret->init()) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	bool SpritePackSoundsPopup::init() {
		auto* pack = PackManager::get().findPack(m_packId);

		// @geode-ignore(unknown-resource)
		if (!Popup::init(300.f, 280.f, "geode.loader/GE_square03.png")) return false;

		this->setTitle(pack ? fmt::format("{} - Sounds", pack->meta.setName) : "Pack Not Found");

		auto size = this->m_mainLayer->getContentSize();
		float midX = size.width / 2.f;

		m_rowScrollLayer = ScrollLayer::create({ SoundRowCell::WIDTH, 220.f });
		m_rowScrollLayer->setID("sounds-row-scroll-layer");
		m_rowScrollLayer->ignoreAnchorPointForPosition(false);
		m_rowScrollLayer->setAnchorPoint({ 0.5f, 0.5f });
		m_rowScrollLayer->setPosition({ midX, size.height / 2.f - 8.f });
		this->m_mainLayer->addChild(m_rowScrollLayer, 1);

		auto scrollBg = CCLayerColor::create();
		scrollBg->setColor({ 0, 0, 0 });
		scrollBg->setOpacity(60);
		scrollBg->ignoreAnchorPointForPosition(false);
		scrollBg->setAnchorPoint({ 0.5f, 0.5f });
		scrollBg->setContentSize({ SoundRowCell::WIDTH + 4.f, m_rowScrollLayer->getContentSize().height + 4.f });
		scrollBg->setPosition(m_rowScrollLayer->getPosition());
		this->m_mainLayer->addChild(scrollBg, 0);

		this->buildRows();

		return true;
	}

	void SpritePackSoundsPopup::buildRows() {
		m_rowScrollLayer->m_contentLayer->removeAllChildren();

		auto* pack = PackManager::get().findPack(m_packId);
		std::vector<SoundRowCell*> cells;

		if (pack) {
			std::vector<std::string> gamemodeNames;
			gamemodeNames.reserve(pack->sounds.gamemodes.size());
			for (auto const& [gm, entries] : pack->sounds.gamemodes) gamemodeNames.push_back(gm);
			std::sort(gamemodeNames.begin(), gamemodeNames.end());

			for (auto const& gamemode : gamemodeNames) {
				auto const& entries = pack->sounds.gamemodes.at(gamemode);

				cells.push_back(SoundRowCell::createHeader(gamemode, cells.size() % 2 == 0));

				std::vector<std::string> entryIds;
				entryIds.reserve(entries.size());
				for (auto const& [entryId, entry] : entries) entryIds.push_back(entryId);
				std::sort(entryIds.begin(), entryIds.end());

				for (auto const& entryId : entryIds) {
					auto const& entry = entries.at(entryId);

					auto* eventCell = SoundRowCell::createEvent(
						entryId, std::vector<std::string>{ describeEntry(entry) },
						settings::isSoundEnabled(m_packId, gamemode, entryId, entry.defaultEnabled),
						cells.size() % 2 == 0, this,
						menu_selector(SpritePackSoundsPopup::onToggleSound),
						menu_selector(SpritePackSoundsPopup::onListen)
					);
					for (CCNode* node : { static_cast<CCNode*>(eventCell->getToggle()), static_cast<CCNode*>(eventCell->getListenButton()) }) {
						node->setUserObject("row-gamemode"_spr, CCString::create(gamemode));
						node->setUserObject("row-event"_spr, CCString::create(entryId));
					}
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
			std::max(totalHeight, SoundRowCell::HEADER_HEIGHT)
		});
		m_rowScrollLayer->moveToTop();
	}

	void SpritePackSoundsPopup::onListen(CCObject* sender) {
		auto* node = static_cast<CCNode*>(sender);
		auto* gmObj = static_cast<CCString*>(node->getUserObject("row-gamemode"_spr));
		auto* idObj = static_cast<CCString*>(node->getUserObject("row-event"_spr));
		if (!gmObj || !idObj) return;
		std::string gamemode = gmObj->getCString();
		std::string entryId = idObj->getCString();

		auto* pack = PackManager::get().findPack(m_packId);
		if (!pack) return;
		auto* entry = pack->sounds.entry(gamemode, entryId);
		if (!entry || entry->files.empty()) return;

		auto& next = m_nextListen[fmt::format("{}/{}", gamemode, entryId)];
		next %= entry->files.size();
		audio::playFile(entry->files[next]);
		next++;
	}

	void SpritePackSoundsPopup::onToggleSound(CCObject* sender) {
		auto* node = static_cast<CCNode*>(sender);
		auto* gmObj = static_cast<CCString*>(node->getUserObject("row-gamemode"_spr));
		auto* evObj = static_cast<CCString*>(node->getUserObject("row-event"_spr));
		if (!gmObj || !evObj) return;
		std::string gamemode = gmObj->getCString();
		std::string eventName = evObj->getCString();

		auto* pack = PackManager::get().findPack(m_packId);
		auto* entry = pack ? pack->sounds.entry(gamemode, eventName) : nullptr;
		bool currentState = settings::isSoundEnabled(m_packId, gamemode, eventName, entry ? entry->defaultEnabled : true);
		settings::setSoundEnabled(m_packId, gamemode, eventName, !currentState);
	}

}
