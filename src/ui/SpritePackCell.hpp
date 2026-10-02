#pragma once

#include "../data/SpritePackTypes.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace playersprites {
	
	class SpritePackCell : public CCLayer {
	protected:
		CCLayerColor* m_background = nullptr;
		CCMenuItemSpriteExtra* m_settingsButton = nullptr;

		bool initForPack(SpritePack const& pack, bool even, CCObject* target, SEL_MenuHandler settingsSelector);
		bool initForError(std::string const& id, std::string const& error, bool even);

	public:
		static constexpr float HEIGHT = 50.f;
		static constexpr float WIDTH = 360.f;

		static SpritePackCell* createForPack(SpritePack const& pack, bool even, CCObject* target, SEL_MenuHandler settingsSelector);
		static SpritePackCell* createForError(std::string const& id, std::string const& error, bool even);
	};

}
