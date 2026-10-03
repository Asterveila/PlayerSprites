#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace playersprites {

	class SettingsRowCell : public CCLayer {
	protected:
		CCLayerColor* m_background = nullptr;
		CCMenuItemToggler* m_toggle = nullptr;

		bool initRow(std::string const& label, bool enabled, bool indented, bool even, CCObject* target, SEL_MenuHandler selector);

	public:
		static constexpr float HEIGHT = 30.f;
		static constexpr float WIDTH = 360.f;

		static SettingsRowCell* create(std::string const& label, bool enabled, bool indented, bool even, CCObject* target, SEL_MenuHandler selector);

		CCMenuItemToggler* getToggle() const { return m_toggle; }
	};

}
