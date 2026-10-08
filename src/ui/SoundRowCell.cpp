#include "SoundRowCell.hpp"

namespace playersprites {

	namespace {
		constexpr float LISTEN_AREA = 30.f;
		constexpr float TOGGLE_AREA = 40.f;

		std::string joinNames(std::vector<std::string> const& names) {
			std::string joined;
			for (size_t i = 0; i < names.size(); ++i) {
				if (i > 0) joined += ", ";
				joined += names[i];
			}
			return joined;
		}
	}

	SoundRowCell* SoundRowCell::createHeader(std::string const& gamemode, bool even) {
		auto ret = new SoundRowCell();
		if (ret->initHeader(gamemode, even)) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	SoundRowCell* SoundRowCell::createEvent(std::string const& eventName, std::vector<std::string> const& fileNames, bool enabled, bool even, CCObject* target, SEL_MenuHandler toggleSelector, SEL_MenuHandler listenSelector) {
		auto ret = new SoundRowCell();
		if (ret->initEvent(eventName, fileNames, enabled, even, target, toggleSelector, listenSelector)) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	bool SoundRowCell::initHeader(std::string const& gamemode, bool even) {
		if (!CCLayer::init()) return false;

		CCSize size = { WIDTH, HEADER_HEIGHT };
		this->setContentSize(size);

		m_background = CCLayerColor::create();
		m_background->setContentSize(size);
		m_background->setOpacity(even ? 50 : 25);
		m_background->setColor({ 0, 0, 0 });
		this->addChild(m_background, -1);

		auto label = CCLabelBMFont::create(fmt::format("{} Sounds", gamemode).c_str(), "goldFont.fnt");
		label->setAnchorPoint({ 0.f, 0.5f });
		label->setPosition({ 10.f, size.height / 2.f });
		label->limitLabelWidth(WIDTH - 20.f, 0.55f, 0.1f);
		this->addChild(label);

		return true;
	}

	bool SoundRowCell::initEvent(std::string const& eventName, std::vector<std::string> const& fileNames, bool enabled, bool even, CCObject* target, SEL_MenuHandler toggleSelector, SEL_MenuHandler listenSelector) {
		if (!CCLayer::init()) return false;

		CCSize size = { WIDTH, EVENT_HEIGHT };
		this->setContentSize(size);

		m_background = CCLayerColor::create();
		m_background->setContentSize(size);
		m_background->setOpacity(even ? 50 : 25);
		m_background->setColor({ 40, 60, 90 });
		this->addChild(m_background, -1);

		float textX = LISTEN_AREA + 6.f;
		float maxTextWidth = WIDTH - textX - TOGGLE_AREA;

		auto nameLabel = CCLabelBMFont::create(eventName.c_str(), "bigFont.fnt");
		nameLabel->setAnchorPoint({ 0.f, 0.5f });
		nameLabel->setPosition({ textX, size.height * 0.66f });
		nameLabel->limitLabelWidth(maxTextWidth, 0.45f, 0.1f);
		this->addChild(nameLabel);

		auto filesLabel = CCLabelBMFont::create(joinNames(fileNames).c_str(), "chatFont.fnt");
		filesLabel->setAnchorPoint({ 0.f, 0.5f });
		filesLabel->setPosition({ textX, size.height * 0.28f });
		filesLabel->setOpacity(190);
		filesLabel->limitLabelWidth(maxTextWidth, 0.4f, 0.1f);
		this->addChild(filesLabel);

		auto menu = CCMenu::create();
		menu->setContentSize(size);
		menu->setPosition({ 0.f, 0.f });
		menu->setAnchorPoint({ 0.f, 0.f });
		this->addChild(menu);

		auto playSpr = CCSprite::createWithSpriteFrameName("GJ_playMusicBtn_001.png");
		playSpr->setScale(0.7f);
		m_listenButton = CCMenuItemSpriteExtra::create(playSpr, target, listenSelector);
		m_listenButton->setPosition({ LISTEN_AREA / 2.f + 4.f, size.height / 2.f });
		menu->addChild(m_listenButton);

		auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
		auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
		m_toggle = CCMenuItemToggler::create(offSpr, onSpr, target, toggleSelector);
		m_toggle->setScale(0.6f);
		m_toggle->setPosition({ size.width - 20.f, size.height / 2.f });
		m_toggle->toggle(enabled);
		menu->addChild(m_toggle);

		return true;
	}

}
