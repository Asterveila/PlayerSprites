#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

namespace playersprites {

	class SpritePackSoundsPopup : public Popup {
	protected:
		std::string m_packId;
		ScrollLayer* m_rowScrollLayer = nullptr;

		std::unordered_map<std::string, size_t> m_nextListen;

		bool init() override;
		void buildRows();

		void onToggleSound(CCObject* sender);
		void onListen(CCObject* sender);

	public:
		static SpritePackSoundsPopup* create(std::string const& packId);
	};

}
