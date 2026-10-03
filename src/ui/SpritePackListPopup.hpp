#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

namespace playersprites {

	class SpritePackListPopup : public Popup {
	protected:
		ScrollLayer* m_packScrollLayer = nullptr;

		bool init() override;
		void buildList();

		void onReload(CCObject* sender);
		void onTogglePack(CCObject* sender);
		void onPackSettings(CCObject* sender);

	public:
		static SpritePackListPopup* create();
	};

}
