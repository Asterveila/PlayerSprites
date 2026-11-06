#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

namespace playersprites {

	class SettingsRowCell;
	class SpritePackSettingsPopup : public Popup {
	protected:
		std::string m_packId;
		ScrollLayer* m_rowScrollLayer = nullptr;
		bool m_showingTaunts = false;
		CCMenuItemToggler* m_eventsToggler = nullptr;
		CCMenuItemToggler* m_tauntsToggler = nullptr;
		std::function<void()> m_onPackDeleted;

		bool init() override;
		void buildRows();
		void buildTauntRows();
		void layoutCells(ScrollLayer* layer, std::vector<SettingsRowCell*> const& cells);
		void setView(bool showTaunts);

		void onToggleClassic(CCObject* sender);
		void onTogglePlatformer(CCObject* sender);
		void onToggleSfx(CCObject* sender);
		void onOpenSounds(CCObject* sender);
		void onToggleGamemode(CCObject* sender);
		void onToggleEvent(CCObject* sender);
		void onExport(CCObject* sender);
		void onShowEvents(CCObject* sender);
		void onShowTaunts(CCObject* sender);
		void onToggleTaunt(CCObject* sender);
		void onDelete(CCObject* sender);

	public:
		static SpritePackSettingsPopup* create(std::string const& packId);

		void setOnPackDeleted(std::function<void()> callback) { m_onPackDeleted = std::move(callback); }
	};

}
