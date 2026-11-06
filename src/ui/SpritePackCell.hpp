#pragma once

#include "../data/SpritePackTypes.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace playersprites {

	class SpritePackCell : public CCLayer {
	protected:
		CCLayerColor* m_background = nullptr;
		CCMenuItemToggler* m_packToggle = nullptr;
		CCMenuItemSpriteExtra* m_infoButton = nullptr;
		CCMenuItemSpriteExtra* m_settingsButton = nullptr;
		CCMenuItemSpriteExtra* m_moveUpButton = nullptr;
		CCMenuItemSpriteExtra* m_moveDownButton = nullptr;

		bool initForPack(SpritePack const& pack, bool even, CCObject* target, SEL_MenuHandler toggleSelector, SEL_MenuHandler infoSelector, SEL_MenuHandler settingsSelector, SEL_MenuHandler moveUpSelector, SEL_MenuHandler moveDownSelector, bool canMoveUp, bool canMoveDown);
		bool initForError(std::string const& id, std::string const& error, bool even);

	public:
		static constexpr float HEIGHT = 50.f;
		static constexpr float WIDTH = 460.f;

		static SpritePackCell* createForPack(SpritePack const& pack, bool even, CCObject* target, SEL_MenuHandler toggleSelector, SEL_MenuHandler infoSelector, SEL_MenuHandler settingsSelector, SEL_MenuHandler moveUpSelector, SEL_MenuHandler moveDownSelector, bool canMoveUp, bool canMoveDown);
		static SpritePackCell* createForError(std::string const& id, std::string const& error, bool even);
	};

}
