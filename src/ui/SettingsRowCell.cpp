#include "SettingsRowCell.hpp"
#include "../visuals/PlayerSprite.hpp"
#include <algorithm>

namespace playersprites {

	namespace {

		constexpr float PREVIEW_BOX = 38.f;
		constexpr float PREVIEW_PADDING = 3.f;
		constexpr float TOGGLE_AREA = 40.f;

		std::string buildTriggeredByLine(AnimEvent const& anim) {
			std::vector<std::string> const triggers =
				anim.triggerOn.empty() ? std::vector<std::string>{ anim.id } : anim.triggerOn;

			std::string joined;
			for (size_t i = 0; i < triggers.size(); ++i) {
				if (i > 0) joined += ", ";
				joined += triggers[i];
			}
			return fmt::format("Triggered by: {}", joined);
		}

	}

	SettingsRowCell* SettingsRowCell::createHeader(std::string const& gamemode, bool enabled, bool even, CCObject* target, SEL_MenuHandler selector) {
		auto ret = new SettingsRowCell();
		if (ret->initHeader(gamemode, enabled, even, target, selector)) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	SettingsRowCell* SettingsRowCell::createEvent(SpritePack const& pack, AnimEvent const& anim, bool enabled, bool even, CCObject* target, SEL_MenuHandler selector) {
		auto ret = new SettingsRowCell();
		if (ret->initEvent(pack, anim, enabled, even, target, selector)) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	bool SettingsRowCell::initHeader(std::string const& gamemode, bool enabled, bool even, CCObject* target, SEL_MenuHandler selector) {
		if (!CCLayer::init()) return false;

		CCSize size = { WIDTH, HEADER_HEIGHT };
		this->setContentSize(size);

		m_background = CCLayerColor::create();
		m_background->setContentSize(size);
		m_background->setOpacity(even ? 50 : 25);
		m_background->setColor({ 0, 0, 0 });
		this->addChild(m_background, -1);

		auto label = CCLabelBMFont::create(fmt::format("{} Animations", gamemode).c_str(), "goldFont.fnt");
		label->setAnchorPoint({ 0.f, 0.5f });
		label->setPosition({ TOGGLE_AREA, size.height / 2.f });
		label->limitLabelWidth(WIDTH - 10.f - TOGGLE_AREA, 0.55f, 0.1f);
		this->addChild(label);

		this->addToggle(enabled, target, selector, 0.6f, true);

		return true;
	}

	bool SettingsRowCell::initEvent(SpritePack const& pack, AnimEvent const& anim, bool enabled, bool even, CCObject* target, SEL_MenuHandler selector) {
		if (!CCLayer::init()) return false;

		CCSize size = { WIDTH, EVENT_HEIGHT };
		this->setContentSize(size);

		m_background = CCLayerColor::create();
		m_background->setContentSize(size);
		m_background->setOpacity(even ? 50 : 25);
		m_background->setColor({ 40, 60, 90 });
		this->addChild(m_background, -1);

		this->addPreview(pack, anim);

		float textX = 10.f + PREVIEW_BOX + 10.f;
		float maxTextWidth = WIDTH - textX - TOGGLE_AREA;

		auto idLabel = CCLabelBMFont::create(anim.id.c_str(), "bigFont.fnt");
		idLabel->setAnchorPoint({ 0.f, 0.5f });
		idLabel->setPosition({ textX, size.height * 0.66f });
		idLabel->limitLabelWidth(maxTextWidth, 0.45f, 0.1f);
		this->addChild(idLabel);

		auto triggersLabel = CCLabelBMFont::create(buildTriggeredByLine(anim).c_str(), "chatFont.fnt");
		triggersLabel->setAnchorPoint({ 0.f, 0.5f });
		triggersLabel->setPosition({ textX, size.height * 0.28f });
		triggersLabel->setOpacity(190);
		triggersLabel->limitLabelWidth(maxTextWidth, 0.4f, 0.1f);
		this->addChild(triggersLabel);

		this->addToggle(enabled, target, selector, 0.7f);

		return true;
	}

	void SettingsRowCell::addToggle(bool enabled, CCObject* target, SEL_MenuHandler selector, float scale, bool atStart) {
		auto size = this->getContentSize();

		auto menu = CCMenu::create();
		menu->setContentSize(size);
		menu->setPosition({ 0.f, 0.f });
		menu->setAnchorPoint({ 0.f, 0.f });
		this->addChild(menu);

		auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
		auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
		m_toggle = CCMenuItemToggler::create(offSpr, onSpr, target, selector);
		m_toggle->setScale(scale);
		atStart ? m_toggle->setPosition({ 20.f, size.height / 2.f }) : m_toggle->setPosition({ size.width - 20.f, size.height / 2.f });
		m_toggle->toggle(enabled);
		menu->addChild(m_toggle);
	}

	void SettingsRowCell::addPreview(SpritePack const& pack, AnimEvent const& anim) {
		auto size = this->getContentSize();
		CCPoint center = { 10.f + PREVIEW_BOX / 2.f, size.height / 2.f };

		auto box = CCLayerColor::create(ccColor4B{ 0, 0, 0, 80 });
		box->setContentSize({ PREVIEW_BOX, PREVIEW_BOX });
		box->setPosition({ center.x - PREVIEW_BOX / 2.f, center.y - PREVIEW_BOX / 2.f });
		this->addChild(box);

		auto frames = PlayerSprite::buildFrames(pack, anim);
		if (frames.empty()) {
			auto missing = CCLabelBMFont::create("?", "bigFont.fnt");
			missing->setScale(0.5f);
			missing->setOpacity(150);
			missing->setPosition(center);
			this->addChild(missing);
			return;
		}

		auto sprite = CCSprite::createWithSpriteFrame(frames[0]);
		auto contentSize = sprite->getContentSize();
		float maxDim = PREVIEW_BOX - PREVIEW_PADDING * 2.f;
		if (contentSize.width > 0.f && contentSize.height > 0.f) {
			sprite->setScale(std::min(maxDim / contentSize.width, maxDim / contentSize.height));
		}
		sprite->setPosition(center);
		this->addChild(sprite);

		if (!anim.singleFrame && frames.size() > 1) {
			auto frameArray = CCArray::createWithCapacity(frames.size());
			for (auto* frame : frames) {
				frameArray->addObject(frame);
			}

			float frameTime = std::max(anim.frameTime.value_or(anim.maxFrameTime.value_or(0.08f)), 0.03f);

			auto animation = CCAnimation::createWithSpriteFrames(frameArray, frameTime);
			sprite->runAction(CCRepeatForever::create(CCAnimate::create(animation)));
		}
	}

}
