#pragma once

#include "../data/SpritePackTypes.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace playersprites {

	class SettingsRowCell : public CCLayer {
	protected:
		CCLayerColor* m_background = nullptr;
		CCMenuItemToggler* m_toggle = nullptr;

		bool initHeader(std::string const& gamemode, bool enabled, bool even, CCObject* target, SEL_MenuHandler selector);
		bool initEvent(SpritePack const& pack, AnimEvent const& anim, bool enabled, bool even, CCObject* target, SEL_MenuHandler selector);

		void addToggle(bool enabled, CCObject* target, SEL_MenuHandler selector, float scale = 0.6f, bool atStart = false);
		void addPreview(SpritePack const& pack, AnimEvent const& anim);

	public:
		static constexpr float WIDTH = 400.f;
		static constexpr float HEADER_HEIGHT = 28.f;
		static constexpr float EVENT_HEIGHT = 46.f;

		static SettingsRowCell* createHeader(std::string const& gamemode, bool enabled, bool even, CCObject* target, SEL_MenuHandler selector);
		static SettingsRowCell* createEvent(SpritePack const& pack, AnimEvent const& anim, bool enabled, bool even, CCObject* target, SEL_MenuHandler selector);

		CCMenuItemToggler* getToggle() const { return m_toggle; }
	};

}
