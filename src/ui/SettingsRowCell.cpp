#include "SettingsRowCell.hpp"

namespace playersprites {

	SettingsRowCell* SettingsRowCell::create(std::string const& label, bool enabled, bool indented, bool even, CCObject* target, SEL_MenuHandler selector) {
		auto ret = new SettingsRowCell();
		if (ret->initRow(label, enabled, indented, even, target, selector)) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	bool SettingsRowCell::initRow(std::string const& label, bool enabled, bool indented, bool even, CCObject* target, SEL_MenuHandler selector) {
		if (!CCLayer::init()) return false;

		CCSize size = { WIDTH, HEIGHT };
		this->setContentSize(size);

		m_background = CCLayerColor::create();
		m_background->setContentSize(size);
		m_background->setOpacity(even ? 50 : 25);
		m_background->setColor(indented ? ccColor3B{ 40, 60, 90 } : ccColor3B{ 0, 0, 0 });
		this->addChild(m_background, -1);

		float indent = indented ? 24.f : 8.f;

		auto labelNode = CCLabelBMFont::create(label.c_str(), indented ? "chatFont.fnt" : "bigFont.fnt");
		labelNode->setAnchorPoint({ 0.f, 0.5f });
		labelNode->setPosition({ indent, size.height / 2.f });
		labelNode->setScale(indented ? 0.45f : 0.5f);
		labelNode->limitLabelWidth(WIDTH - indent - 60.f, indented ? 0.45f : 0.5f, 0.1f);
		this->addChild(labelNode);

		auto menu = CCMenu::create();
		menu->setContentSize(size);
		menu->setPosition({ 0.f, 0.f });
		menu->setAnchorPoint({ 0.f, 0.f });
		this->addChild(menu);

		auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
		auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
		m_toggle = CCMenuItemToggler::create(offSpr, onSpr, target, selector);
		m_toggle->setScale(0.6f);
		m_toggle->setPosition({ size.width - 20.f, size.height / 2.f });
		m_toggle->toggle(enabled);
		menu->addChild(m_toggle);

		return true;
	}

}
