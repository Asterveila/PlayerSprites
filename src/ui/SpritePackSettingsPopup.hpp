#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

namespace playersprites {
	
	class SpritePackSettingsPopup : public Popup {
	protected:
		std::string m_packId;
		ScrollLayer* m_rowScrollLayer = nullptr;

		bool init() override;
		void buildRows();

		void onToggleClassic(CCObject* sender);
		void onTogglePlatformer(CCObject* sender);
		void onToggleGamemode(CCObject* sender);
		void onToggleEvent(CCObject* sender);
		void onOpenFolder(CCObject* sender);

	public:
		static SpritePackSettingsPopup* create(std::string const& packId);
	};

}
