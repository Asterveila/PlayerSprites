#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace playersprites {

	class SoundRowCell : public CCLayer {
	protected:
		CCLayerColor* m_background = nullptr;
		CCMenuItemToggler* m_toggle = nullptr;
		CCMenuItemSpriteExtra* m_listenButton = nullptr;

		bool initHeader(std::string const& gamemode, bool even);
		bool initEvent(std::string const& eventName, std::vector<std::string> const& fileNames, bool enabled, bool even, CCObject* target, SEL_MenuHandler toggleSelector, SEL_MenuHandler listenSelector);

	public:
		static constexpr float WIDTH = 260.f;
		static constexpr float HEADER_HEIGHT = 25.f;
		static constexpr float EVENT_HEIGHT = 38.f;

		static SoundRowCell* createHeader(std::string const& gamemode, bool even);
		static SoundRowCell* createEvent(std::string const& eventName, std::vector<std::string> const& fileNames, bool enabled, bool even, CCObject* target, SEL_MenuHandler toggleSelector, SEL_MenuHandler listenSelector);

		CCMenuItemToggler* getToggle() const { return m_toggle; }
		CCMenuItemSpriteExtra* getListenButton() const { return m_listenButton; }
	};

}
